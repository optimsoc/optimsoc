/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

(3) Neither the name of the Multicore Association nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



/////////////////////////////////////////////////////////////////////////////
// File: transport_sm.c
// 
// Description: This file contains a shared memory implemenation of MCAPI.
//  This is not intended to be a high-performance real-world implementation.
//  Rather it is just intended as a prototype so that we can "kick the tires"
//  as the spec is being developed.
//
// Authors: Michele Reese
//
/////////////////////////////////////////////////////////////////////////////



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mcapi.h> /*for cached_domain */
#include <mcapi_trans.h> /* the transport API */
#include "transport_sm.h"
#include <mca_utils.h> /*for crc32 */

#include <string.h> /* for memcpy */
#include <sys/ipc.h> /* for ftok */
#include <pwd.h> /*for get uid */
#include <assert.h> /* for assertions */
#include <sched.h> /* for sched_yield */
#include <stdlib.h> /* for exit */

  
/* NOTE:
   This code is a bit brittle in that you have to be very careful when you 
   exit a function with respect to locking and unlocking the database.  You
   also have to be careful that if you lock the database you unlock it when
   you are finished.  If you modify the code and put a return before you
   have released the lock, then other nodes will not be able to access the
   database.  You can dial up the debug level and lock/unlock matching 
   will be turned on for you (see access_database_pre/post).  Also some
   functions assume the database is locked at the time they are called.  
   These functions have an _have_lock in their name as well as a comment to 
   indicate this.
*/

/* NOTE:
   The memory layout for the database is not compact.  It just uses 
   simple multi-dimensional arrays of size MCAPI_MAX_*.  A more memory efficient
   and more flexible mechanism might be to use a heap memory manager for the 
   shared memory segment.    
*/

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                   Constants and defines                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define MAGIC_NUM 0xdeadcafe
#define MCAPI_VALID_MASK 0x80000000
  
  
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                   Function prototypes (private)                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////   
  mcapi_boolean_t mcapi_trans_access_database_pre (uint32_t handle,mcapi_boolean_t exclusive);

  mcapi_boolean_t mcapi_trans_access_database_post (uint32_t handle,mcapi_boolean_t exclusive);
  
  mcapi_boolean_t mcapi_trans_decode_request_handle(mcapi_request_t* request,
                                                    uint16_t* r);

  mcapi_boolean_t mcapi_trans_decode_handle (uint32_t handle, 
                                                      uint16_t* domain_index,
                                                      uint16_t *node_index,
                                                      uint16_t *endpoint_index);

  uint32_t mcapi_trans_encode_handle (uint16_t domain_index,
                                               uint16_t node_index,
                                               uint16_t endpoint_index);
  void mcapi_trans_signal_handler ( int sig );
  
  mcapi_boolean_t mcapi_trans_send_have_lock (uint16_t sd, uint16_t sn,uint16_t se, 
                                             uint16_t rd, uint16_t rn,uint16_t re, 
                                             const char* buffer, 
                                             size_t buffer_size,
                                             uint64_t scalar);
  
  mcapi_boolean_t mcapi_trans_recv_have_lock (uint16_t rd, uint16_t rn, uint16_t re, 
                                             void** buffer, 
                                             size_t buffer_size, 
                                             size_t* received_size,
                                             mcapi_boolean_t blocking,
                                             uint64_t* scalar);
  
  void mcapi_trans_recv_have_lock_ (uint16_t rd, uint16_t rn, uint16_t re, 
                                   void** buffer, 
                                   size_t buffer_size,
                                   size_t* received_size,
                                   int qindex,
                                   uint64_t* scalar);

  mcapi_boolean_t mcapi_trans_endpoint_get_have_lock (mcapi_endpoint_t *e, 
                                                     mcapi_domain_t domain_id,
                                                     mcapi_uint_t node_num, 
                                                     mcapi_uint_t port_num);
  
  void mcapi_trans_open_channel_have_lock (uint16_t d,uint16_t n, uint16_t e);

  void mcapi_trans_close_channel_have_lock (uint16_t d,uint16_t n, uint16_t e);

  void mcapi_trans_connect_channel_have_lock (mcapi_endpoint_t send_endpoint, 
                                             mcapi_endpoint_t receive_endpoint,
                                             channel_type type);
  
  mcapi_boolean_t mcapi_trans_reserve_request_have_lock(int* r);
  
  mcapi_boolean_t setup_request_have_lock (mcapi_endpoint_t* endpoint,
                                          mcapi_request_t* request,
                                          mcapi_status_t* mcapi_status,
                                          mcapi_boolean_t completed, 
                                          size_t size,
                                          void** buffer,
                                          mcapi_request_type type,
                                          mcapi_uint_t node_num, 
                                          mcapi_uint_t port_num, 
                                          mcapi_domain_t domain_num,
                                          int r);

  void check_receive_request_have_lock (mcapi_request_t *request);
  
  void cancel_receive_request_have_lock (mcapi_request_t *request);

  void check_get_endpt_request_have_lock (mcapi_request_t *request);

  void check_open_channel_request_have_lock (mcapi_request_t *request);
  
  inline mcapi_boolean_t mcapi_trans_whoami (mcapi_node_t* node_id,
                                                      uint32_t* n_index,
                                                      mcapi_domain_t* domain_id,
                                                      uint32_t* d_index);
  
  void mcapi_trans_display_state_have_lock (void* handle);
  
  void mcapi_trans_yield_have_lock();

  /* queue management */
  void print_queue (queue q);
  int mcapi_trans_pop_queue (queue *q);
  int mcapi_trans_push_queue (queue *q);
  mcapi_boolean_t mcapi_trans_empty_queue (queue q); 
  mcapi_boolean_t mcapi_trans_full_queue (queue q);
  void mcapi_trans_compact_queue (queue* q);
  mcapi_boolean_t mcapi_trans_endpoint_create_(mcapi_endpoint_t* ep, 
                                               mcapi_domain_t domain_id,
                                               mcapi_node_t node_num, 
                                               mcapi_uint_t port_num,
                                               mcapi_boolean_t anonymous);


#define mcapi_assert(x) MCAPI_ASSERT(x,__LINE__);
  void MCAPI_ASSERT(mcapi_boolean_t condition,unsigned line) {
    if (!condition) {
      fprintf(stderr,"INTERNAL ERROR: MCAPI failed assertion (transport_sm.c:%d) shutting down\n",line);
      mcapi_trans_signal_handler(SIGABRT);
      exit(1);
    }
  }

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                   Data                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
  
/* public globals (use thread-local-storage) */
  
#define TLS __thread
  
  /* do not put the database in thread-local-storage, it causes the pointer to be nulled when a 
     system call occurs (like cntrl-c) and gdb can't see into it */
  mcapi_database* mcapi_db = NULL; /* our shared memory addr for our have_lock database */
  TLS pid_t mcapi_pid = -1;  
  TLS pthread_t mcapi_tid;
  TLS unsigned mcapi_nindex = 0;
  TLS unsigned mcapi_dindex = 0;
  TLS int mcapi_debug = 0;
  TLS uint32_t global_rwl = 0; /* the global database lock */
  TLS mcapi_node_t mcapi_node_num = 0;
  TLS mcapi_domain_t mcapi_domain_id = 0;
  TLS int locked = 0;
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                   mcapi_trans API                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
  
  /***************************************************************************
  NAME: mcapi_trans_get_node_num
  DESCRIPTION: gets the node_num (not the transport's node index!)
  PARAMETERS: node_num: the node_num pointer to be filled in
  RETURN VALUE: boolean indicating success (the node num was found) or failure 
   (couldn't find the node num).
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_get_node_num(mcapi_node_t* node) 
  {
    uint32_t d,n;
    mcapi_domain_t domain_id;
    
    mcapi_dprintf(1,"mcapi_trans_get_node_num(&node_dummy);");
    
    return mcapi_trans_whoami(node,&n,&domain_id,&d);
  }
  
  /***************************************************************************
  NAME: mcapi_trans_get_domain_num
  DESCRIPTION: gets the domain_num (not the transport's node index!)
  PARAMETERS: domain_num: the domain_num pointer to be filled in
  RETURN VALUE: boolean indicating success (the node num was found) or failure
   (couldn't find the domain num).
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_get_domain_num(mcapi_domain_t* domain)
  {
    uint32_t d,n;
    mcapi_node_t node;
    
    return mcapi_trans_whoami(&node,&n,domain,&d);
  }
  
  /****************************_***********************************************
  mcapi_trans_remove_request_have_lock
  DESCRIPTION: Removes request from array
  PARAMETERS:
    r - request index
  RETURN VALUE: TRUE/FALSE indicating if the request has removed.
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_remove_request_have_lock(int r) {	/* by etem */

  	int temp_empty_head_index;
    mcapi_boolean_t rc = MCAPI_FALSE;
  	indexed_array_header *header = &mcapi_db->request_reserves_header;

  	if (header->full_head_index != -1) {
  		if (header->full_head_index == r) {
  			header->full_head_index = header->array[header->full_head_index].next_index;
  		}
  		if (header->array[r].next_index != -1) {
  			header->array[header->array[r].next_index].prev_index = header->array[r].prev_index;
  		}
  		if (header->array[r].prev_index != -1) {
  			header->array[header->array[r].prev_index].next_index = header->array[r].next_index;
  		}
  		temp_empty_head_index = header->empty_head_index;
  		header->empty_head_index = r;
  		header->array[header->empty_head_index].next_index = temp_empty_head_index;
  		if (temp_empty_head_index != -1) {
  			header->array[temp_empty_head_index].prev_index = header->empty_head_index;
  		}

  		header->curr_count--;
  		rc = MCAPI_TRUE;
  	}
  	return rc; // if rc=false, then there is no request available (array is empty)
  }
  /***************************************************************************
  NAME: mcapi_trans_reserve_request
  DESCRIPTION: Reserves an entry in the requests array
  PARAMETERS: *r - request index pointer
  RETURN VALUE: T/F
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_reserve_request_have_lock(int *r) {	/* by etem */

  	int temp_full_head_index;
    mcapi_boolean_t rc = MCAPI_FALSE;
    
  	indexed_array_header *header = &mcapi_db->request_reserves_header;
    
  	if (header->empty_head_index != -1) {
  		*r = header->empty_head_index;
      mcapi_db->requests[*r].valid = MCAPI_TRUE;
  		temp_full_head_index = header->full_head_index;
  		header->full_head_index = header->empty_head_index;
  		header->empty_head_index = header->array[header->empty_head_index].next_index;
  		header->array[header->empty_head_index].prev_index = -1;
  		header->array[header->full_head_index].next_index = temp_full_head_index;
  		header->array[header->full_head_index].prev_index = -1;
  		if (temp_full_head_index != -1) {
  			header->array[temp_full_head_index].prev_index = header->full_head_index;
  		}
  		header->curr_count++;
  		rc = MCAPI_TRUE;
  	}
    return rc;
  }

  /***************************************************************************
  NAME: mcapi_trans_init_indexed_array_have_lock
  DESCRIPTION: initializes indexed array
  PARAMETERS:
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_init_indexed_array_have_lock() {	/* by etem */
  	int i;
		
    mcapi_db->request_reserves_header.curr_count = 0;
		mcapi_db->request_reserves_header.max_count = MCAPI_MAX_REQUESTS;
		mcapi_db->request_reserves_header.empty_head_index = 0;
		mcapi_db->request_reserves_header.full_head_index = -1;
		for (i = 0; i < MCAPI_MAX_REQUESTS; i++) {
			mcapi_db->request_reserves_header.array[i].next_index = i + 1;
			mcapi_db->request_reserves_header.array[i].prev_index = i - 1;
		}
		mcapi_db->request_reserves_header.array[MCAPI_MAX_REQUESTS - 1].next_index = -1;
		mcapi_db->request_reserves_header.array[0].prev_index = -1;

  }

  /***************************************************************************
  NAME: mcapi_trans_initialize
  DESCRIPTION: sets up the semaphore and shared memory if necessary
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_initialize (mcapi_domain_t domain_id,
                                          mcapi_node_t node_id,
                                          const mcapi_node_attributes_t* node_attrs) 
  {
    
    /* I've re-written the initialize and finalize routines a few times.  This seems to work
       most of the time.  However, we do still have a race condition.  mtapi_test3 in the
       regression is currently turned off since it runs into the race condition sometimes.
       The race occurs when one thread is initializing while another is finalizing.  A prize
       to whomever can find the problem and get that test to reliably pass (the test does nothing
       - each thread/node just initializes and then finalizes).
    */
    
    mcapi_boolean_t rc = MCAPI_TRUE;
    void* shm_addr;
    int d = 0;
    int n = 0;
    register struct passwd *pw;
    register uid_t uid;
    char buff[128];
    int i; 
    mcapi_boolean_t use_uid = MCAPI_TRUE;
    int key;
    
    /* initialize this node in the mrapi layer so that this pid/tid will be recognized */
    /* this will also create a semaphore for us to use */
    
    /* fixme: the mrapi transport should check and use uid if requested for creating the semaphore */
    if (!transport_sm_initialize(domain_id,node_id,&global_rwl)) {
      mcapi_dprintf(1,"ERROR: mcapi_transport_sm_initialize FAILED\n");
      return MCAPI_FALSE;
    }
    
    /* lock the database */
    if (!mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE)) { 
      return MCAPI_FALSE; 
    }
    
    if (use_uid) { 
      uid = geteuid ();
      pw = getpwuid (uid);
      if (pw) {
        //printf ("pw=%s\n",pw->pw_name);
      } else {
        fprintf (stderr," cannot find username for UID %u\n", (unsigned) uid);
      }    
      memset(buff,0,sizeof(buff));
      strcat(buff,pw->pw_name);
      strcat(buff,"_mcapi");
      key = 0;
      key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
      mcapi_dprintf(1,"using shared memory %s, key = 0x%x\n",buff,key);
    } else {
      key = ftok("/dev/null",'c');
    }
    
    /* create the shared memory (it may already exist) */
    // mcapi_cached_domain = domain_id;
    
    if (transport_sm_create_shared_mem(&shm_addr,key,sizeof(mcapi_database))) {
      mcapi_dprintf(1," Requesting %lu bytes of shared memory using key=%x\n",sizeof(mcapi_database),key);
      mcapi_dprintf(1," using defaults: MCAPI_MAX_MSG_SIZE=%u, MCAPI_MAX_PKT_SIZE=%u, MCAPI_MAX_ENDPOINTS=%u, MCAPI_MAX_ATTRIBUTES=%u, MCAPI_MAX_CHANNELS=%u, MCA_MAX_NODES=%u, MCAPI_MAX_BUFFERS=%u, MCAPI_MAX_QUEUE_ELEMENTS=%u",
                    MCAPI_MAX_MSG_SIZE, MCAPI_MAX_PKT_SIZE, MCAPI_MAX_ENDPOINTS, MCAPI_MAX_ATTRIBUTES, 
                    MCAPI_MAX_CHANNELS, MCA_MAX_NODES, MCAPI_MAX_BUFFERS,MCAPI_MAX_QUEUE_ELEMENTS);
      /* setup the database pointer */
      mcapi_db = (mcapi_database*)shm_addr;
      /* the memory is not zeroed for us, go through and set all nodes/domains to invalid */
      for (d=0; d<MCA_MAX_DOMAINS; d++) {
        mcapi_db->domains[d].valid = MCAPI_FALSE;
        for (n=0; n<MCA_MAX_NODES; n++) {
          mcapi_db->domains[d].nodes[n].valid = MCAPI_FALSE;
        }
      }
      mcapi_trans_init_indexed_array_have_lock();	/* by etem */  
    } else if (transport_sm_get_shared_mem(&shm_addr,key,sizeof(mcapi_database))) {
      mcapi_dprintf(1,"Attaching to shared memory (it already existed)\n");
      /* setup the database pointer */
      mcapi_db = (mcapi_database*)shm_addr;
    } else {
      fprintf(stderr,"FAILED: Couldn't get and attach to shared memory (is the database too large?).\n");
      rc = MCAPI_FALSE;
    } 
    
    
    mcapi_dprintf(1," using MCAPI database in shared memory at %p\n",mcapi_db);  
    
    /*  add the node/domain to the database */
    if (rc) {
      /* first see if this domain already exists */
      for (d = 0; d < MCA_MAX_DOMAINS; d++) {
        if (mcapi_db->domains[d].domain_id == domain_id) {
          break;
        }
      }
      if (d == MCA_MAX_DOMAINS) {
        /* it didn't exist so find the first available entry */
        for (d = 0; d < MCA_MAX_DOMAINS; d++) {
          if (mcapi_db->domains[d].valid == MCAPI_FALSE) {
            break;
          }
        }
      }
      if (d != MCA_MAX_DOMAINS) {
        /* now find an available node index...*/
        for (n = 0; n < MCA_MAX_NODES; n++) {
          /* Even though initialized() is checked by mcapi, we have to check again here because 
             initialized() and initalize() are  not atomic at the top layer */
          if ((mcapi_db->domains[d].nodes[n].valid )&& 
              (mcapi_db->domains[d].nodes[n].node_num == node_id)) {
            /* this node already exists for this domain */
            rc = MCAPI_FALSE;
            mcapi_dprintf(1,"This node (%d) already exists for this domain(%d)",node_id,domain_id);
            break; 
          }
        }
        if (n == MCA_MAX_NODES) {
          /* it didn't exist so find the first available entry */
          for (n = 0; n < MCA_MAX_NODES; n++) {
            if (mcapi_db->domains[d].nodes[n].valid == MCAPI_FALSE)
              break;
          }
        }
      } else {
        /* we didn't find an available domain index */
        mcapi_dprintf(1,"You have hit MCA_MAX_DOMAINS, either use less domains or reconfigure with more domains");
        rc = MCAPI_FALSE;
      }
      if (n == MCA_MAX_NODES) {
        /* we didn't find an available node index */
        mcapi_dprintf(1,"You have hit MCA_MAX_NODES, either use less nodes or reconfigure with more nodes.");
        rc = MCAPI_FALSE;
      }
    }
    
    if (rc) {
      if (n < MCA_MAX_NODES) {
        /* add the caller to the database*/
        /* set the domain */
        mcapi_db->domains[d].domain_id = domain_id;
        mcapi_db->domains[d].valid = MCAPI_TRUE;
        /* set the node */ 
        mcapi_pid = getpid();
        mcapi_tid = pthread_self();
        mcapi_nindex = n;
        mcapi_node_num = node_id;
        mcapi_domain_id = domain_id;
        mcapi_dindex = d;
        mcapi_db->domains[d].nodes[n].valid = MCAPI_TRUE;  
        mcapi_db->domains[d].nodes[n].node_num = node_id;
        mcapi_db->domains[d].nodes[n].pid = mcapi_pid;
        mcapi_db->domains[d].nodes[n].tid = mcapi_tid;
        mcapi_db->domains[d].num_nodes++;
        /* set the node attributes */
        if (node_attrs != NULL) {
          memcpy(&mcapi_db->domains[d].nodes[n].attributes,
                 node_attrs,
                 sizeof(mcapi_node_attributes_t));
        }
        /* initialize the attribute size for the only attribute we support */
        mcapi_db->domains[d].nodes[n].attributes.entries[MCAPI_NODE_ATTR_TYPE_REGULAR].bytes=
          sizeof(mcapi_node_attr_type_t);

        for (i = 0; i < MCAPI_MAX_ENDPOINTS; i++) {
          /* zero out all the endpoints */
          memset (&mcapi_db->domains[d].nodes[n].node_d.endpoints[i],0,sizeof(endpoint_entry));
        }
      } 
      
      
      mcapi_dprintf(1,"registering mcapi signal handlers\n");
      /* register signal handlers so that we can still clean up resources 
         if an interrupt occurs 
         http://www.gnu.org/software/libtool/manual/libc/Sigaction-Function-Example.html
      */
      struct sigaction new_action, old_action;
      
      /* Set up the structure to specify the new action. */
      new_action.sa_handler = mcapi_trans_signal_handler;
      sigemptyset (&new_action.sa_mask);
      new_action.sa_flags = 0;
      
      sigaction (SIGINT, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGINT] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGINT, &new_action, NULL);
      
      sigaction (SIGHUP, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGHUP] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGHUP, &new_action, NULL);
      
      sigaction (SIGILL, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGILL] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGILL, &new_action, NULL);  
      
      sigaction (SIGSEGV, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGSEGV] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGSEGV, &new_action, NULL);
      
      sigaction (SIGTERM, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGTERM] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGTERM, &new_action, NULL);
      
      sigaction (SIGFPE, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGFPE] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGFPE, &new_action, NULL);  
      
      sigaction (SIGABRT, NULL, &old_action);
      mcapi_db->domains[d].nodes[n].signals[SIGABRT] = old_action;
      if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGABRT, &new_action, NULL);
      
    } 
    
    mcapi_dprintf(1, "mcapi_trans_initialize complete.  domain=%u, node=%u added\n",
                  domain_id,node_id);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_node_init_attributes
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_node_init_attributes(
                                        mcapi_node_attributes_t* mcapi_node_attributes,
                                        mcapi_status_t* mcapi_status
                                        ) {
    mcapi_boolean_t rc = MCAPI_TRUE;
    /* default values are all 0 */
    memset(mcapi_node_attributes,0,sizeof(mcapi_node_attributes));
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_node_get_attribute
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_node_get_attribute(
                                                 mcapi_domain_t domain_id,
                                                 mcapi_node_t node_id,
                                                 mcapi_uint_t attribute_num,
                                                 void* attribute,
                                                 size_t attribute_size,
                                                 mcapi_status_t* mcapi_status) {
    mcapi_boolean_t rc = MCAPI_FALSE;
    mcapi_boolean_t found_node = MCAPI_FALSE;
    mcapi_boolean_t found_domain = MCAPI_FALSE;
    uint32_t d,n;
    mcapi_domain_t my_domain_id;
    mcapi_node_t my_node_id;
    size_t size;
    
    if (!mcapi_trans_whoami(&my_node_id,&n,&my_domain_id,&d)) {
      *mcapi_status = MCAPI_ERR_NODE_NOTINIT;
    } else if (attribute_num != MCAPI_NODE_ATTR_TYPE_REGULAR) {
      /* only the node_attr_type attribute is currently supported */
      *mcapi_status = MCAPI_ERR_ATTR_NOTSUPPORTED;
    } else {
      /* lock the database */
      mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
      
      // look for the <domain,node>
      for (d = 0; ((d < MCA_MAX_DOMAINS) && (found_domain == MCAPI_FALSE)); d++) {
        if (mcapi_db->domains[d].domain_id == domain_id) {
          found_domain = MCAPI_TRUE;
          for (n = 0; ((n < MCA_MAX_NODES) &&  (found_node == MCAPI_FALSE)); n++) {
            if (mcapi_db->domains[d].nodes[n].node_num == node_id) { 
              found_node = MCAPI_TRUE;
              if (!mcapi_db->domains[d].valid) {
                *mcapi_status = MCAPI_ERR_DOMAIN_INVALID;
              } else if (!mcapi_db->domains[d].nodes[n].valid) {
                *mcapi_status = MCAPI_ERR_NODE_INVALID;
              } else {
                size = mcapi_db->domains[d].nodes[n].attributes.entries[attribute_num].bytes;
                 if (size != attribute_size) {
                  *mcapi_status = MCAPI_ERR_ATTR_SIZE;
                 } else {
                   memcpy(attribute,
                       &mcapi_db->domains[d].nodes[n].attributes.entries[attribute_num].attribute_d,
                       size);
                   rc = MCAPI_TRUE;
                }
              }
            }
          }
        }
      }  
      if (!found_domain) {
        *mcapi_status = MCAPI_ERR_DOMAIN_INVALID;
      } else if (!found_node) {
        *mcapi_status = MCAPI_ERR_NODE_INVALID;
      }
      /* unlock the database */
      mcapi_assert (mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    }
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_node_set_attribute
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_node_set_attribute(
                                      mcapi_node_attributes_t* mcapi_node_attributes,
                                      mcapi_uint_t attribute_num,
                                      const void* attribute,
                                      size_t attribute_size,
                                      mcapi_status_t* mcapi_status
                                      ){
    mcapi_boolean_t rc = MCAPI_FALSE;
    
    if (attribute_num != MCAPI_NODE_ATTR_TYPE_REGULAR) {
      /* only the node_attr_type attribute is currently supported */
      *mcapi_status = MCAPI_ERR_ATTR_NOTSUPPORTED;
    } else if (attribute_size != sizeof(mcapi_node_attr_type_t) ) {
      *mcapi_status = MCAPI_ERR_ATTR_SIZE;
    } else {
      rc = MCAPI_TRUE;
      /* copy the attribute into the attributes data structure */
      memcpy(&mcapi_node_attributes->entries[attribute_num].attribute_d,
             attribute,
             attribute_size);
    }
    return rc;
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_finalize
  DESCRIPTION: cleans up the semaphore and shared memory resources.
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_finalize () {
    mcapi_boolean_t last_man_standing = MCAPI_TRUE;
    mcapi_boolean_t last_man_standing_for_this_process = MCAPI_TRUE;
    mcapi_boolean_t rc = MCAPI_TRUE;
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t i;
    mcapi_node_t node_id=0;
    mcapi_domain_t domain_id=0;
    
    if (mcapi_db == NULL) { return MCAPI_FALSE;}
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_whoami(&node_id,&n,&domain_id,&d)); 
    
    mcapi_dprintf(1,"mcapi_trans_finalize(domain=%u node=%u);",domain_id,node_id);
    
    /* mark myself as invalid and see if there are any other valid nodes in the system */
    mcapi_db->domains[d].nodes[n].valid = MCAPI_FALSE;
    
    for (d = 0; d < MCA_MAX_DOMAINS; d++) {
      for (i = 0; i < MCA_MAX_NODES; i++) {
        if ( mcapi_db->domains[d].nodes[i].valid) {
          last_man_standing = MCAPI_FALSE;
          if  ( mcapi_db->domains[d].nodes[i].pid == mcapi_pid) {
            last_man_standing_for_this_process = MCAPI_FALSE;
          }
        }
      }
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    /* finalize this node at the mrapi layer */
    /* if there are no other nodes, mrapi_finalize will free up our resources 
       (semaphore and shared memory db) */
    rc = transport_sm_finalize(last_man_standing,last_man_standing_for_this_process,MCAPI_TRUE,global_rwl);
    
    /* transport_sm_finalize will have detached from the shared memory so
       null out our db pointer */
    if (last_man_standing_for_this_process) {
      mcapi_dprintf(1,"mcapi_trans_finalize: deleting mcapi_db\n");
      mcapi_db = NULL;
    } 
    
    return rc;
  }
  
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   mcapi_trans API: error checking routines               //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  /***************************************************************************
  NAME: mcapi_trans_channel_type
  DESCRIPTION: Given an endpoint, returns the type of channel (if any)
   associated with it.
  PARAMETERS: endpoint: the endpoint to be checked
  RETURN VALUE: the type of the channel (pkt,scalar or none)
  ***************************************************************************/
  channel_type mcapi_trans_channel_type (mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    int rc;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    rc = mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type;
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_send_endpoint
  DESCRIPTION:checks if the given endpoint is a send endpoint
  PARAMETERS: endpoint: the endpoint to be checked
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_send_endpoint (mcapi_endpoint_t endpoint) 
  {
    uint16_t d,n,e;
    int rc = MCAPI_TRUE;
    
    mcapi_dprintf(2,"mcapi_trans_send_endpoint(0x%x);",endpoint);
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    if ((mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected) &&
        (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.recv_endpt == endpoint)) {
      /* this endpoint has already been marked as a receive endpoint */
      mcapi_dprintf(2,"mcapi_trans_send_endpoint ERROR: this endpoint (0x%x) has already been connected as a receive endpoint",
                    endpoint); 
      rc = MCAPI_FALSE;
    } 
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_recv_endpoint 
  DESCRIPTION:checks if the given endpoint can be or is already a receive endpoint 
  PARAMETERS: endpoint: the endpoint to be checked
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_recv_endpoint (mcapi_endpoint_t endpoint) 
  {
    uint16_t d,n,e;
    int rc = MCAPI_TRUE;
    
    mcapi_dprintf(2,"mcapi_trans_recv_endpoint(0x%x);",endpoint);
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    if ((mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected) &&
        (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.send_endpt == endpoint)) {
      /* this endpoint has already been marked as a send endpoint */ 
      mcapi_dprintf(2,"mcapi_trans_recv_endpoint ERROR: this endpoint (0x%x) has already been connected as a send endpoint",
                    endpoint); 
      rc = MCAPI_FALSE;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_port
  DESCRIPTION:checks if the given port_num is a valid port_num for this system
  PARAMETERS: port_num: the port num to be checked
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_port(mcapi_uint_t port_num)
  {
    return MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_node
  DESCRIPTION: checks if the given node_num is a valid node_num for this system
  PARAMETERS: node_num: the node num to be checked
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_node(mcapi_uint_t node_num)
  {
    return MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_valid_endpoint
  DESCRIPTION: checks if the given endpoint handle refers to a valid endpoint
  PARAMETERS: endpoint
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_endpoint (mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    mcapi_dprintf(2,"mcapi_trans_valid_endpoint(0x%x);",endpoint);
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    if (mcapi_trans_decode_handle(endpoint,&d,&n,&e)) {
      rc = ( mcapi_db->domains[d].valid && mcapi_db->domains[d].nodes[n].valid && 
             mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid);
    }
    
    mcapi_dprintf(2,"mcapi_trans_valid_endpoint endpoint=0x%llx (database indices: n=%u,e=%u) rc=%u",
                  (unsigned long long)endpoint,n,e,rc);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_endpoint_exists
  DESCRIPTION: checks if an endpoint has been created for this port id
  PARAMETERS: port id
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_endpoint_exists (mcapi_domain_t domain_id, 
                                               uint32_t port_num)
  {
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t i;
    mcapi_node_t node_id;
    int rc = MCAPI_FALSE;
    
    if (port_num == MCAPI_PORT_ANY) {
      return rc;
    }
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_whoami(&node_id,&n,&domain_id,&d));
    
    /* Note: we can't just iterate for i < num_endpoints because endpoints can
       be deleted which would fragment the endpoints array. */
    for (i = 0; i < MCAPI_MAX_ENDPOINTS; i++) {
      if (mcapi_db->domains[d].nodes[n].node_d.endpoints[i].valid && 
          mcapi_db->domains[d].nodes[n].node_d.endpoints[i].port_num == port_num) {
        rc = MCAPI_TRUE;
        break;
      }
    }  
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_valid_endpoints
  DESCRIPTION: checks if the given endpoint handles refer to valid endpoints
  PARAMETERS: endpoint1, endpoint2
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_endpoints (mcapi_endpoint_t endpoint1, 
                                               mcapi_endpoint_t endpoint2)
  {
    uint16_t d1,n1,e1;
    uint16_t d2,n2,e2;
    mcapi_boolean_t rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    if (mcapi_trans_decode_handle(endpoint1,&d1,&n1,&e1) && 
        mcapi_db->domains[d1].nodes[n1].node_d.endpoints[e1].valid &&
        mcapi_trans_decode_handle(endpoint2,&d2,&n2,&e2) && 
        mcapi_db->domains[d2].nodes[n2].node_d.endpoints[e2].valid) {
      rc = MCAPI_TRUE;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_recv_isopen
  DESCRIPTION:checks if the channel is open for a given handle 
  PARAMETERS: receive_handle
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_pktchan_recv_isopen (mcapi_pktchan_recv_hndl_t receive_handle)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(receive_handle,&d,&n,&e));
    rc = (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_send_isopen
  DESCRIPTION:checks if the channel is open for a given handle 
  PARAMETERS: send_handle
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_pktchan_send_isopen (mcapi_pktchan_send_hndl_t send_handle)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(send_handle,&d,&n,&e));
    rc =  (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_recv_isopen
  DESCRIPTION:checks if the channel is open for a given handle 
  PARAMETERS: receive_handle
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_sclchan_recv_isopen (mcapi_sclchan_recv_hndl_t receive_handle)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(receive_handle,&d,&n,&e));
    rc = (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_send_isopen
  DESCRIPTION:checks if the channel is open for a given handle 
  PARAMETERS: send_handle
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_sclchan_send_isopen (mcapi_sclchan_send_hndl_t send_handle)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(send_handle,&d,&n,&e));
    rc = (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_channel_isopen
  DESCRIPTION:checks if a channel is open for a given endpoint 
  PARAMETERS: endpoint
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_endpoint_channel_isopen (mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
  
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    rc =  (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_isowner 
  DESCRIPTION:checks if the given endpoint is owned by the calling node
  PARAMETERS: endpoint
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_endpoint_isowner (mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    mcapi_node_t node_num=0;
    int rc = MCAPI_FALSE;
    
    mcapi_assert(mcapi_trans_get_node_num(&node_num));
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));  
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    rc = ((mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid) && 
          (mcapi_db->domains[d].nodes[n].node_num == node_num));
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_channel_connected 
  DESCRIPTION:checks if the given endpoint channel is connected 
  PARAMETERS: endpoint
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_channel_connected (mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    rc = ((mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid) && 
          (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected));
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_compatible_endpoint_attributes  
  DESCRIPTION:checks if the given endpoints have compatible attributes
  PARAMETERS: send_endpoint,recv_endpoint 
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_compatible_endpoint_attributes (mcapi_endpoint_t send_endpoint, 
                                                              mcapi_endpoint_t recv_endpoint)
  { 
    /* FIXME: (errata A3) currently un-implemented */
    return MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_pktchan_send_handle
  DESCRIPTION:checks if the given pkt channel send handle is valid
  PARAMETERS: handle
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_pktchan_send_handle( mcapi_pktchan_send_hndl_t handle)
  {
    uint16_t d,n,e;
    channel_type type;
    
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf (2,"mcapi_trans_valid_pktchan_send_handle (0x%x);",handle);
    
    type =MCAPI_PKT_CHAN;
    if (mcapi_trans_decode_handle(handle,&d,&n,&e)) {
      if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type == type) {
        rc = MCAPI_TRUE;
      } else {
        mcapi_dprintf(2,"mcapi_trans_valid_pktchan_send_handle node=%u,port=%u returning false channel_type != MCAPI_PKT_CHAN",
                      mcapi_db->domains[d].nodes[n].node_num,
                      mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
      }
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_pktchan_recv_handle
  DESCRIPTION:checks if the given pkt channel recv handle is valid 
  PARAMETERS: handle
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_pktchan_recv_handle( mcapi_pktchan_recv_hndl_t handle)
  {
    uint16_t d,n,e;
    channel_type type;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf (2,"mcapi_trans_valid_pktchan_recv_handle (0x%x);",handle);
    
    type = MCAPI_PKT_CHAN;
    if (mcapi_trans_decode_handle(handle,&d,&n,&e)) {
      if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type == type) {
        rc = MCAPI_TRUE;
      } else {
        mcapi_dprintf(2,"mcapi_trans_valid_pktchan_recv_handle node=%u,port=%u returning false channel_type != MCAPI_PKT_CHAN",
                      mcapi_db->domains[d].nodes[n].node_num,
                      mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
      }
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_sclchan_send_handle
  DESCRIPTION: checks if the given scalar channel send handle is valid 
  PARAMETERS: handle
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_sclchan_send_handle( mcapi_sclchan_send_hndl_t handle)
  {
    uint16_t d,n,e;
    channel_type type;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf (2,"mcapi_trans_valid_sclchan_send_handle (0x%x);",handle);
    
    type = MCAPI_SCL_CHAN;
    if (mcapi_trans_decode_handle(handle,&d,&n,&e)) {
      if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type == type) { 
        rc = MCAPI_TRUE;
      } else {
        mcapi_dprintf(2,"mcapi_trans_valid_sclchan_send_handle node=%u,port=%u returning false channel_type != MCAPI_SCL_CHAN",
                      mcapi_db->domains[d].nodes[n].node_num,
                      mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
      }
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_sclchan_recv_handle
  DESCRIPTION:checks if the given scalar channel recv handle is valid 
  PARAMETERS: 
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_sclchan_recv_handle( mcapi_sclchan_recv_hndl_t handle)
  {
    uint16_t d,n,e;
    channel_type type;
    
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf (2,"mcapi_trans_valid_sclchan_recv_handle (0x%x);",handle);
    
    type= MCAPI_SCL_CHAN;
    if (mcapi_trans_decode_handle(handle,&d,&n,&e)) {
      if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type == type) {
        rc = MCAPI_TRUE;
      } else {
        mcapi_dprintf(2,"mcapi_trans_valid_sclchan_recv_handle node=%u,port=%u returning false channel_type != MCAPI_SCL_CHAN",
                      mcapi_db->domains[d].nodes[n].node_num,
                      mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
      }
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_initialized
  DESCRIPTION: checks if the calling node has called initialize
  PARAMETERS: 
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_initialized ()
  {  
    uint32_t d,n;
    mcapi_node_t node;
    mcapi_domain_t domain;
    mcapi_boolean_t rc = MCAPI_FALSE;
    
    mcapi_dprintf (1,"mcapi_trans_initialized();");
   
    /* lock the database */
    if (!mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE)) {
      return MCAPI_FALSE;
    }
        
    rc = mcapi_trans_whoami(&node,&n,&domain,&d);
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_num_endpoints
  DESCRIPTION: returns the current number of endpoints for the calling node
  PARAMETERS:  none
  RETURN VALUE: num_endpoints
  ***************************************************************************/
  mcapi_uint32_t mcapi_trans_num_endpoints()
  {
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t rc = 0;
    mcapi_node_t node_id=0;
    mcapi_domain_t domain_id=0;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_whoami(&node_id,&n,&domain_id,&d));
   
    mcapi_dprintf (2,"mcapi_trans_num_endpoints (0x%x);",domain_id);
 
    rc = mcapi_db->domains[d].nodes[n].node_d.num_endpoints;
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }

  /***************************************************************************
  NAME:mcapi_trans_valid_priority
  DESCRIPTION:checks if the given priority level is valid
  PARAMETERS: priority
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_priority(mcapi_priority_t priority)
  {
    return ((priority >=0) && (priority <= MCAPI_MAX_PRIORITY));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_connected
  DESCRIPTION: checks if the given endpoint is connected
  PARAMETERS: endpoint
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_connected(mcapi_endpoint_t endpoint)
  {
    mcapi_boolean_t rc = MCAPI_FALSE;
    uint16_t d,n,e;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf (2,"mcapi_trans_connected (0x%x);",endpoint);
    
    rc = (mcapi_trans_decode_handle(endpoint,&d,&n,&e) && 
          (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type != MCAPI_NO_CHAN));
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    return rc;
  }
  
  /***************************************************************************
  NAME:valid_status_param
  DESCRIPTION: checks if the given status is a valid status parameter
  PARAMETERS: status
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_status_param (mcapi_status_t* mcapi_status)
  {
    return (mcapi_status != NULL);
  }
  
  /***************************************************************************
  NAME:mcapi_trans_valid_version_param
  DESCRIPTION: checks if the given version is a valid version parameter
  PARAMETERS: version
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_version_param (mcapi_info_t* mcapi_version)
  {
    return (mcapi_version != NULL);
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_valid_buffer_param
  DESCRIPTION:checks if the given buffer is a valid buffer parameter
  PARAMETERS: buffer
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_buffer_param (void* buffer)
  {
    return (buffer != NULL);
  }
  
  
  /***************************************************************************
  NAME: mcapi_trans_valid_request_handle
  DESCRIPTION:checks if the given request handle is valid 
        This is the parameter test/wait/cancel pass in to be processed.
  PARAMETERS: request
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_request_handle (mcapi_request_t* request)
  {
    uint16_t r;
    return (mcapi_trans_decode_request_handle(request,&r));  
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_valid_size_param
  DESCRIPTION: checks if the given size is a valid size parameter
  PARAMETERS: size
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_valid_size_param (size_t* size)
  {
    return (size != NULL);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   mcapi_trans API: endpoints                             //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_create_
  DESCRIPTION:create endpoint <node_num,port_num> and return it's handle 
  PARAMETERS: 
       ep - the endpoint to be filled in
       port_num - the port id (only valid if !anonymous)
       anonymous - whether or not this should be an anonymous endpoint
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_endpoint_create (mcapi_endpoint_t* ep, 
                                               mcapi_uint_t port_num,
                                               mcapi_boolean_t anonymous)
  {
    uint32_t domain_index = 0;
    uint32_t node_index = 0;
    uint32_t i, endpoint_index;
    mcapi_boolean_t rc = MCAPI_FALSE;
    mcapi_domain_t domain_id2;
    mcapi_node_t node_num2;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
  
    mcapi_dprintf (1,"mcapi_trans_endpoint_create (0x%x,%u);",port_num,anonymous);
    
    mcapi_assert(mcapi_trans_whoami(&node_num2,&node_index,&domain_id2,&domain_index));
    
    
    /* make sure there's room - mcapi should have already checked this */
    mcapi_assert (mcapi_db->domains[domain_index].nodes[node_index].node_d.num_endpoints < MCAPI_MAX_ENDPOINTS);
    
    /* create the endpoint */
    /* find the first available endpoint index */
    endpoint_index = MCAPI_MAX_ENDPOINTS;
    for (i = 0; i < MCAPI_MAX_ENDPOINTS; i++) {
      if (! mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[i].valid) {
        endpoint_index = i;
        break;
      }
    }
    
    if (endpoint_index < MCAPI_MAX_ENDPOINTS) {
      
      /* initialize the endpoint entry*/  
      mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[endpoint_index].valid = MCAPI_TRUE;
      mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[endpoint_index].port_num = port_num;
      mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[endpoint_index].open = MCAPI_FALSE;
      mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[endpoint_index].anonymous = anonymous;
      mcapi_db->domains[domain_index].nodes[node_index].node_d.endpoints[endpoint_index].num_attributes = 0;
      
      mcapi_db->domains[domain_index].nodes[node_index].node_d.num_endpoints++; 
      
      /* set the handle */
      *ep = mcapi_trans_encode_handle (domain_index,node_index,endpoint_index);

      rc = MCAPI_TRUE;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }

  /***************************************************************************
  NAME:mcapi_trans_endpoint_get_attribute
  DESCRIPTION:
  PARAMETERS:
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_endpoint_get_attribute(
                                          mcapi_endpoint_t endpoint,
                                          mcapi_uint_t attribute_num,
                                          void* attribute,
                                          size_t attribute_size,
                                          mcapi_status_t* mcapi_status)
  {
    
    uint16_t d,n,e;
    int* attr = attribute;
    
    *mcapi_status = MCAPI_ERR_ATTR_NUM;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    
    if (attribute_num == MCAPI_ENDP_ATTR_NUM_RECV_BUFFERS) {
      *attr = MCAPI_MAX_QUEUE_ENTRIES - 
        mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.num_elements;
      *mcapi_status = MCAPI_SUCCESS;
    }
    
    /* unlock the database */
    mcapi_assert (mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
  }
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_set_attribute
  DESCRIPTION:
  PARAMETERS:
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_endpoint_set_attribute(
                                          mcapi_endpoint_t endpoint,
                                          mcapi_uint_t attribute_num,
                                          const void* attribute,
                                          size_t attribute_size,
                                          mcapi_status_t* mcapi_status)
  {
    
    fprintf(stderr,"WARNING: setting endpoint attributes not implemented\n");
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_get_i
  DESCRIPTION:non-blocking get endpoint for the given <node_num,port_num> 
  PARAMETERS: 
     endpoint - the endpoint handle to be filled in
     node_num - the node id
     port_num - the port id
     request - the request handle to be filled in when the task is complete
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_endpoint_get_i( mcapi_endpoint_t* endpoint,
                                   mcapi_domain_t domain_id,
                                   mcapi_uint_t node_num,
                                   mcapi_uint_t port_num, 
                                   mcapi_request_t* request,
                                   mcapi_status_t* mcapi_status)
  {
    
    mcapi_boolean_t valid =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_TRUE : MCAPI_FALSE; 
    mcapi_boolean_t completed = MCAPI_FALSE;
    int r;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have an available request entry*/
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (valid) {
        /* try to get the endpoint */
        if (mcapi_trans_endpoint_get_have_lock (endpoint,domain_id,node_num,port_num)) {
          completed = MCAPI_TRUE;
        } 
      }
      
      mcapi_assert(setup_request_have_lock(endpoint,request,mcapi_status,completed,0,NULL,GET_ENDPT,node_num,port_num,domain_id,r));
    }
    
    /* unlock the database */
    mcapi_assert (mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_get_have_lock
  DESCRIPTION:get endpoint for the given <node_num,port_num> 
  PARAMETERS: 
     endpoint - the endpoint handle to be filled in
     node_num - the node id
     port_num - the port id
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_endpoint_get_have_lock (mcapi_endpoint_t *ep, 
                                                     mcapi_domain_t domain_id, 
                                                     mcapi_uint_t node_num, 
                                                     mcapi_uint_t port_num) 
  {
    uint32_t d,n,e;
    
    /* the database should already be locked */
    assert(locked == 1);
    mcapi_dprintf(3,"mcapi_trans_endpoint_get_have_lock(&ep,%u,%u,%u);",domain_id,node_num,port_num);
    
    // look for the endpoint <domain,node,port>
    for (d = 0; d < MCA_MAX_DOMAINS; d++) {
      if (mcapi_db->domains[d].valid && 
          (mcapi_db->domains[d].domain_id == domain_id)) {
        for (n = 0; n < MCA_MAX_NODES; n++) {
          if (mcapi_db->domains[d].nodes[n].valid &&
              (mcapi_db->domains[d].nodes[n].node_num == node_num)) { 
            // iterate over all the endpoints on this node
            for (e = 0; e < mcapi_db->domains[d].nodes[n].node_d.num_endpoints; e++) {
              if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid &&
                  (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num == port_num)) {
                /* we found it, return the handle */
                *ep = mcapi_trans_encode_handle (d,n,e);
                return MCAPI_TRUE;
              }
            }
          }
        }
      }
    }
    
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_endpoint_get
  DESCRIPTION:blocking get endpoint for the given <node_num,port_num> 
  PARAMETERS: 
     endpoint - the endpoint handle to be filled in
     node_num - the node id
     port_num - the port id
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  void mcapi_trans_endpoint_get(mcapi_endpoint_t *e, 
                                mcapi_domain_t domain_id,
                                mcapi_uint_t node_num, 
                                mcapi_uint_t port_num)
  {
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    mcapi_dprintf(1,"mcapi_trans_endpoint_get d=%u,n=%u,p=%u",domain_id,node_num,port_num);
    
    while(!mcapi_trans_endpoint_get_have_lock (e,domain_id,node_num,port_num)) {
      /* yield */
      mcapi_dprintf(5,"mcapi_trans_endpoint_get - attempting to yield");
      mcapi_trans_yield_have_lock();
    }
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME: mcapi_trans_endpoint_delete
  DESCRIPTION:delete the given endpoint
  PARAMETERS: endpoint
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_endpoint_delete( mcapi_endpoint_t endpoint)
  {
    uint16_t d,n,e;
    
    mcapi_dprintf(1,"mcapi_trans_endpoint_delete(0x%x);",endpoint);
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(endpoint,&d,&n,&e));
    
    mcapi_dprintf(2,"mcapi_trans_endpoint_delete_have_lock node_num=%u, port_num=%u",
                  mcapi_db->domains[d].nodes[n].node_num,mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
    
    /* remove the endpoint */
    mcapi_db->domains[d].nodes[n].node_d.num_endpoints--;
    /* zero out the old endpoint entry in the shared memory database */
    memset (&mcapi_db->domains[d].nodes[n].node_d.endpoints[e],0,sizeof(endpoint_entry));
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   mcapi_trans API: messages                              //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  /***************************************************************************
  NAME: mcapi_trans_msg_send
  DESCRIPTION: sends a connectionless message from one endpoint to another (blocking)
  PARAMETERS: 
     send_endpoint - the sending endpoint's handle
     receive_endpoint - the receiving endpoint's handle
     buffer - the user supplied buffer
     buffer_size - the size in bytes of the buffer
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_msg_send( mcapi_endpoint_t  send_endpoint, 
                                        mcapi_endpoint_t  receive_endpoint, 
                                        const char* buffer, 
                                        size_t buffer_size)
  {
    mcapi_request_t request;
    mcapi_status_t status = MCAPI_SUCCESS;
    size_t size;
    
    /* use non-blocking followed by wait */
    do {
      mcapi_trans_msg_send_i (send_endpoint,receive_endpoint,buffer,buffer_size,&request,&status);
    } while (status == MCAPI_ERR_REQUEST_LIMIT);
    
    mcapi_trans_wait (&request,&size,&status,MCA_INFINITE);
    
    if (status == MCAPI_SUCCESS) {
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_msg_send_i
  DESCRIPTION: sends a connectionless message from one endpoint to another (non-blocking)
  PARAMETERS: 
     send_endpoint - the sending endpoint's handle
     receive_endpoint - the receiving endpoint's handle
     buffer - the user supplied buffer
     buffer_size - the size in bytes of the buffer
     request - the request handle to be filled in when the task is complete
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_msg_send_i( mcapi_endpoint_t  send_endpoint, 
                               mcapi_endpoint_t  receive_endpoint, 
                               const char* buffer, 
                               size_t buffer_size, 
                               mcapi_request_t* request,
                               mcapi_status_t* mcapi_status) 
  {
    int r;
    uint16_t sd,sn,se;
    uint16_t rd,rn,re;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    mcapi_dprintf(1,"mcapi_trans_msg_send_i (0x%x,0x%x,buffer,%u,&request,&status);",send_endpoint,receive_endpoint,buffer_size);
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have an available request entry*/
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        completed = MCAPI_TRUE; /* sends complete immediately */
        mcapi_assert(mcapi_trans_decode_handle(send_endpoint,&sd,&sn,&se));
        mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));
        mcapi_assert (mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.channel_type == MCAPI_NO_CHAN);
        mcapi_assert (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_NO_CHAN);
        
        if (!mcapi_trans_send_have_lock (sd,sn,se,rd,rn,re,buffer,buffer_size,0)) {
          /* assume couldn't get a buffer */
          *mcapi_status = MCAPI_ERR_MEM_LIMIT;
        }
      }
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,buffer_size,NULL,SEND,0,0,0,r));
      /* the non-blocking request succeeded, when they call test/wait they will see the status of the send */
      *mcapi_status = MCAPI_SUCCESS; 
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_msg_recv
  DESCRIPTION:receives a message from this endpoints receive queue (blocking)
  PARAMETERS: 
     receive_endpoint - the receiving endpoint
     buffer - the user supplied buffer to copy the message into
     buffer_size - the size of the user supplied buffer
     received_size - the actual size of the message received
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_msg_recv( mcapi_endpoint_t  receive_endpoint,  
                                        char* buffer, 
                                        size_t buffer_size, 
                                        size_t* received_size)
  {
    mcapi_request_t request;
    mcapi_status_t status = MCAPI_SUCCESS;
    
    mcapi_dprintf(2,"mcapi_trans_msg_recv re=0x%x",receive_endpoint);
    
    /* use non-blocking followed by wait */
    do {
      mcapi_trans_msg_recv_i (receive_endpoint,buffer,buffer_size,&request,&status);
    } while (status == MCAPI_ERR_REQUEST_LIMIT);
    
    mcapi_trans_wait (&request,received_size,&status,MCA_INFINITE);
    
    if (status == MCAPI_SUCCESS) {
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
    
  }
  
  /***************************************************************************
  NAME:mcapi_trans_msg_recv_i
  DESCRIPTION:receives a message from this endpoints receive queue (non-blocking)
  PARAMETERS: 
     receive_endpoint - the receiving endpoint
     buffer - the user supplied buffer to copy the message into
     buffer_size - the size of the user supplied buffer
     received_size - the actual size of the message received
     request - the request to be filled in when the task is completed.
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE indicating success or failure
  ***************************************************************************/
  void mcapi_trans_msg_recv_i( mcapi_endpoint_t  receive_endpoint,  char* buffer, 
                               size_t buffer_size, mcapi_request_t* request,
                               mcapi_status_t* mcapi_status) 
  {
    uint16_t rd,rn,re;
    int r;
    size_t received_size = 0;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_msg_recv_i(0x%x,buffer,%u,&request,&status);",receive_endpoint,buffer_size);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));
      
        mcapi_assert (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_NO_CHAN);
        
        if (mcapi_trans_recv_have_lock(rd,rn,re,(void*)&buffer,buffer_size,&received_size,MCAPI_FALSE,NULL)) {
          completed = MCAPI_TRUE;
          buffer_size = received_size;
        }   
      }
      
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,buffer_size,(void**)((void*)&buffer),RECV,0,0,0,r));
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME: mcapi_trans_msg_available
  DESCRIPTION: counts the number of messages in the endpoints receive queue
  PARAMETERS:  endpoint
  RETURN VALUE: the number of messages in the queue
  ***************************************************************************/
  mcapi_uint_t mcapi_trans_msg_available( mcapi_endpoint_t receive_endpoint)
  {
    uint16_t rd,rn,re;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_msg_available(0x%x);",receive_endpoint);
    mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));   
    rc = mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements;
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   mcapi_trans API: packet channels                       //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  /***************************************************************************
  NAME:mcapi_trans_pktchan_connect_i
  DESCRIPTION: connects a packet channel
  PARAMETERS: 
    send_endpoint - the sending endpoint handle
    receive_endpoint - the receiving endpoint handle
    request - the request to be filled in when the task is complete
    mcapi_status -
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_pktchan_connect_i( mcapi_endpoint_t  send_endpoint, 
                                      mcapi_endpoint_t  receive_endpoint, 
                                      mcapi_request_t* request,
                                      mcapi_status_t* mcapi_status)
  {
    
    int r;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {
        mcapi_trans_connect_channel_have_lock (send_endpoint,receive_endpoint,MCAPI_PKT_CHAN);
        completed = MCAPI_TRUE;
      }
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME: mcapi_trans_pktchan_recv_open_i
  DESCRIPTION: opens the receive endpoint on a packet channel
  PARAMETERS:     
    recv_handle - the receive channel handle to be filled in
    receive_endpoint - the receiving endpoint handle
    request - the request to be filled in when the task is complete
    mcapi_status
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_pktchan_recv_open_i( mcapi_pktchan_recv_hndl_t* recv_handle, 
                                        mcapi_endpoint_t receive_endpoint, 
                                        mcapi_request_t* request,
                                        mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t rd,rn,re;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_pktchan_recv_open_i (recv_handle,0x%x,&request,&status);",receive_endpoint);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {
        mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));
        
        mcapi_trans_open_channel_have_lock (rd,rn,re);
        
        /* fill in the channel handle */
        *recv_handle = mcapi_trans_encode_handle(rd,rn,re);
        
        
        /* has the channel been connected yet? */
        if ( mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_PKT_CHAN) {
          completed = MCAPI_TRUE;
        }
        
        mcapi_dprintf(2,"mcapi_trans_pktchan_recv_open_i (node_num=%u,port_num=%u) handle=0x%x",
                      mcapi_db->domains[rd].nodes[rn].node_num,
                      mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].port_num,
                      *recv_handle); 
      }
      
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,0,NULL,OPEN_PKTCHAN,0,0,0,r));
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    } 
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  } 
  
  
  /***************************************************************************
  NAME: mcapi_trans_pktchan_send_open_i
  DESCRIPTION: opens the send endpoint on a packet channel
  PARAMETERS:     
    send_handle - the send channel handle to be filled in
    receive_endpoint - the receiving endpoint handle
    request - the request to be filled in when the task is complete
    mcapi_status
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_pktchan_send_open_i( mcapi_pktchan_send_hndl_t* send_handle, 
                                        mcapi_endpoint_t send_endpoint,
                                        mcapi_request_t* request,
                                        mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t sd,sn,se;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_pktchan_send_open_i,send_handle,0x%x,&request,&status);",send_endpoint);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(send_endpoint,&sd,&sn,&se));
        
        /* mark the endpoint as open */
        mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].open = MCAPI_TRUE;
        
        /* fill in the channel handle */
        *send_handle = mcapi_trans_encode_handle(sd,sn,se);
        
        /* has the channel been connected yet? */
        if ( mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.channel_type == MCAPI_PKT_CHAN) {
          completed = MCAPI_TRUE;
        }
        
        mcapi_dprintf(2," mcapi_trans_pktchan_send_open_i (node_num=%u,port_num=%u) handle=0x%x",
                      mcapi_db->domains[sd].nodes[sn].node_num,
                      mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].port_num,
                      *send_handle);
      }
      
      mcapi_assert(setup_request_have_lock(&send_endpoint,request,mcapi_status,completed,0,NULL,OPEN_PKTCHAN,0,0,0,r));
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_send_i
  DESCRIPTION: sends a packet on a packet channel (non-blocking)
  PARAMETERS: 
    send_handle - the send channel handle
    buffer - the buffer
    size - the size in bytes of the buffer
    request - the request handle to be filled in when the task is complete
    mcapi_status -
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_pktchan_send_i( mcapi_pktchan_send_hndl_t send_handle, 
                                   const void* buffer, size_t size, 
                                   mcapi_request_t* request,
                                   mcapi_status_t* mcapi_status)
  {
    uint16_t sd,sn,se,rd,rn,re;
    int r;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE; 
    
    mcapi_dprintf(1,"mcapi_trans_pktchan_send_i(0x%x,buffer,%u,&request,&status);",send_handle,size);
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {  
        mcapi_assert(mcapi_trans_decode_handle(send_handle,&sd,&sn,&se));   
        mcapi_assert(mcapi_trans_decode_handle(mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.recv_endpt,&rd,&rn,&re));
        
        if (!mcapi_trans_send_have_lock (sd,sn,se,rd,rn,re,(char*)buffer,size,0)) {
          *mcapi_status = MCAPI_ERR_MEM_LIMIT;
        }
        completed = MCAPI_TRUE;
      }
      mcapi_assert(setup_request_have_lock(&send_handle,request,mcapi_status,completed,size,NULL,SEND,0,0,0,r));
      /* the non-blocking request succeeded, when they call test/wait they will see the status of the send */
      *mcapi_status = MCAPI_SUCCESS; 
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_send
  DESCRIPTION: sends a packet on a packet channel (blocking)
  PARAMETERS: 
    send_handle - the send channel handle
    buffer - the buffer
    size - the size in bytes of the buffer
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_pktchan_send( mcapi_pktchan_send_hndl_t send_handle, 
                                            const void* buffer, 
                                            size_t size)
  {
    mcapi_request_t request;
    mcapi_status_t status = MCAPI_SUCCESS;
    
    /* use non-blocking followed by wait */
    do {
      mcapi_trans_pktchan_send_i (send_handle,buffer,size,&request,&status);
    } while (status == MCAPI_ERR_REQUEST_LIMIT);
    
    mcapi_trans_wait (&request,&size,&status,MCA_INFINITE);
    
    if (status == MCAPI_SUCCESS) {
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_recv_i
  DESCRIPTION: receives a packet on a packet channel (non-blocking)
  PARAMETERS: 
    receive_handle - the send channel handle
    buffer - a pointer to a pointer to a buffer 
    request - the request handle to be filled in when the task is complete
    mcapi_status -
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_pktchan_recv_i( mcapi_pktchan_recv_hndl_t receive_handle,  
                                   void** buffer, 
                                   mcapi_request_t* request,
                                   mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t rd,rn,re;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    size_t size = MCAPI_MAX_PKT_SIZE; 
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      mcapi_dprintf(1,"mcapi_trans_pktchan_recv_i(0x%x,&buffer,&request,&status);",receive_handle,size);
      
      if (!completed) {  
        mcapi_assert(mcapi_trans_decode_handle(receive_handle,&rd,&rn,&re));
        
        /* *buffer will be filled in the with a ptr to an mcapi buffer */
        *buffer = NULL;
        if (mcapi_trans_recv_have_lock (rd,rn,re,buffer,MCAPI_MAX_PKT_SIZE,&size,MCAPI_FALSE,NULL)) {
          completed = MCAPI_TRUE;
        }
      }
      
      mcapi_assert(setup_request_have_lock(&receive_handle,request,mcapi_status,completed,size,buffer,RECV,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_recv
  DESCRIPTION: receives a packet on a packet channel (blocking)
  PARAMETERS: 
    send_handle - the send channel handle
    buffer - the buffer
    received_size - the size in bytes of the buffer
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE (only returns MCAPI_FALSE if it couldn't get a buffer)
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_pktchan_recv( mcapi_pktchan_recv_hndl_t receive_handle, 
                                            void** buffer, 
                                            size_t* received_size)
  {                         
    mcapi_request_t request;
    mcapi_status_t status = MCAPI_SUCCESS;
    
    /* use non-blocking followed by wait */
    do {
      mcapi_trans_pktchan_recv_i (receive_handle,buffer,&request,&status);
    } while (status == MCAPI_ERR_REQUEST_LIMIT);
    
    mcapi_trans_wait (&request,received_size,&status,MCA_INFINITE);
    
    if (status == MCAPI_SUCCESS) {
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_pktchan_available
  DESCRIPTION: counts the number of elements in the endpoint receive queue
    identified by the receive handle.
  PARAMETERS: receive_handle - the receive channel handle
  RETURN VALUE: the number of elements in the receive queue
  ***************************************************************************/
  mcapi_uint_t mcapi_trans_pktchan_available( mcapi_pktchan_recv_hndl_t receive_handle)
  {
    uint16_t rd,rn,re;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_pktchan_available(0x%x);",receive_handle);
    
    mcapi_assert(mcapi_trans_decode_handle(receive_handle,&rd,&rn,&re)); 
    rc = mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements;
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_free
  DESCRIPTION: frees the given buffer
  PARAMETERS: buffer pointer
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure (buffer not found)
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_pktchan_free( void* buffer)
  {
    
    int rc = MCAPI_TRUE;
    buffer_entry* b_e;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_pktchan_free(buffer);");
    /* optimization - just do pointer arithmetic on the buffer pointer to get
       the base address of the buffer_entry structure. */
    b_e = buffer-(sizeof(b_e->scalar)+sizeof(b_e->size)+sizeof(b_e->magic_num)+4);
    if (b_e->magic_num == MAGIC_NUM) {
      memset(b_e,0,sizeof(buffer_entry));
    } else {
      /* didn't find the buffer */
      rc = MCAPI_FALSE;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_recv_close_i
  DESCRIPTION: non-blocking close of the receiving end of the packet channel
  PARAMETERS: receive_handle
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_pktchan_recv_close_i( mcapi_pktchan_recv_hndl_t  receive_handle,
                                         mcapi_request_t* request, mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t rd,rn,re;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      mcapi_dprintf(1,"mcapi_trans_pktchan_recv_close_i (0x%x,&request,&status);",receive_handle);
      
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(receive_handle,&rd,&rn,&re));
        mcapi_trans_close_channel_have_lock (rd,rn,re);
        completed = MCAPI_TRUE;    
      }
      mcapi_assert(setup_request_have_lock(&receive_handle,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_pktchan_send_close_i
  DESCRIPTION: non-blocking close of the sending end of the packet channel
  PARAMETERS: receive_handle
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_pktchan_send_close_i( mcapi_pktchan_send_hndl_t  send_handle,
                                         mcapi_request_t* request,mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t sd,sn,se;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {  
      mcapi_dprintf(1,"mcapi_trans_pktchan_send_close_i (0x%x,&request,&status);",send_handle);
      
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(send_handle,&sd,&sn,&se));
        mcapi_trans_close_channel_have_lock (sd,sn,se);
        completed = MCAPI_TRUE;     
      }
      
      mcapi_assert(setup_request_have_lock(&send_handle,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   mcapi_trans API: scalar channels                       //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_connect_i
  DESCRIPTION: connects a scalar channel between the given two endpoints
  PARAMETERS: 
      send_endpoint - the sending endpoint
      receive_endpoint - the receiving endpoint
      request - the request
      mcapi_status - the status
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_sclchan_connect_i( mcapi_endpoint_t  send_endpoint, 
                                      mcapi_endpoint_t  receive_endpoint, 
                                      mcapi_request_t* request,
                                      mcapi_status_t* mcapi_status)
  {
    int r;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {
        mcapi_trans_connect_channel_have_lock (send_endpoint,receive_endpoint,MCAPI_SCL_CHAN);
        completed = MCAPI_TRUE;
      }
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
      
    } else {
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME: mcapi_trans_sclchan_recv_open_i
  DESCRIPTION: opens the receive endpoint on a packet channel
  PARAMETERS:     
    recv_handle - the receive channel handle to be filled in
    receive_endpoint - the receiving endpoint handle
    request - the request to be filled in when the task is complete
    mcapi_status
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_sclchan_recv_open_i( mcapi_sclchan_recv_hndl_t* recv_handle, 
                                        mcapi_endpoint_t receive_endpoint, 
                                        mcapi_request_t* request,
                                        mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t rd,rn,re;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_recv_open_i(recv_handle,0x%x,&request,&status);",receive_endpoint);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));
        
        mcapi_trans_open_channel_have_lock (rd,rn,re);
        
        /* fill in the channel handle */
        *recv_handle = mcapi_trans_encode_handle(rd,rn,re);
        
        /* has the channel been connected yet? */
        if ( mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_SCL_CHAN){
          completed = MCAPI_TRUE;
        }
        
        mcapi_dprintf(2,"mcapi_trans_sclchan_recv_open_i (node_num=%u,port_num=%u) handle=0x%x",
                      mcapi_db->domains[rd].nodes[rn].node_num,
                      mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].port_num,
                      *recv_handle);
      }
      
      mcapi_assert(setup_request_have_lock(&receive_endpoint,request,mcapi_status,completed,0,NULL,OPEN_SCLCHAN,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  
  /***************************************************************************
  NAME: mcapi_trans_sclchan_send_open_i
  DESCRIPTION: opens the receive endpoint on a packet channel
  PARAMETERS:     
    send_handle - the receive channel handle to be filled in
    receive_endpoint - the receiving endpoint handle
    request - the request to be filled in when the task is complete
    mcapi_status
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_sclchan_send_open_i( mcapi_sclchan_send_hndl_t* send_handle, 
                                        mcapi_endpoint_t  send_endpoint, 
                                        mcapi_request_t* request,
                                        mcapi_status_t* mcapi_status)
  {
    int r;
    uint16_t sd,sn,se;
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_send_open_i(send_handle,0x%x,&request,&status);",send_endpoint);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(send_endpoint,&sd,&sn,&se));
        
        /* mark the endpoint as open */
        mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].open = MCAPI_TRUE;
        
        /* fill in the channel handle */
        *send_handle = mcapi_trans_encode_handle(sd,sn,se);
        
        /* has the channel been connected yet? */
        if ( mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.channel_type == MCAPI_SCL_CHAN){
          completed = MCAPI_TRUE;
        }
        
        mcapi_dprintf(2," mcapi_trans_sclchan_send_open_i (node_num=%u,port_num=%u) handle=0x%x",
                      mcapi_db->domains[sd].nodes[sn].node_num,
                      mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].port_num,
                      *send_handle);
      }
      
      mcapi_assert(setup_request_have_lock(&send_endpoint,request,mcapi_status,completed,0,NULL,OPEN_SCLCHAN,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_send
  DESCRIPTION: sends a packet on a packet channel (blocking)
  PARAMETERS: 
    send_handle - the send channel handle
    buffer - the buffer
    size - the size in bytes of the buffer
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_sclchan_send( mcapi_sclchan_send_hndl_t send_handle,  
                                            uint64_t dataword, 
                                            uint32_t size)
  {  
    uint16_t sd,sn,se,rd,rn,re;
    int rc = MCAPI_FALSE;
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_send(0x%x,0x%x,%u);",send_handle,dataword,size);
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_assert(mcapi_trans_decode_handle(send_handle,&sd,&sn,&se));
    mcapi_assert(mcapi_trans_decode_handle(mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.recv_endpt,&rd,&rn,&re));
    
    rc = mcapi_trans_send_have_lock (sd,sn,se,rd,rn,re,NULL,size,dataword); 
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_recv
  DESCRIPTION: receives a packet on a packet channel (blocking)
  PARAMETERS: 
    send_handle - the send channel handle
    buffer - the buffer
    received_size - the size in bytes of the buffer
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE (only returns MCAPI_FALSE if it couldn't get a buffer)
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_sclchan_recv( mcapi_sclchan_recv_hndl_t receive_handle,
                                            uint64_t *data,uint32_t size)
  {
    uint16_t rd,rn,re;
    size_t received_size;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"uint64_t data;");
    mcapi_dprintf(1,"mcapi_trans_sclchan_send(0x%x,&data,%u);",receive_handle,size);
    
    mcapi_assert(mcapi_trans_decode_handle(receive_handle,&rd,&rn,&re));
    
    
    if (mcapi_trans_recv_have_lock (rd,rn,re,NULL,size,&received_size,MCAPI_TRUE,data) &&
        received_size == size) {
      rc = MCAPI_TRUE;
    }
   
    /* FIXME: (errata A2) if size != received_size then we shouldn't remove the item from the
       endpoints receive queue */
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_sclchan_available
  DESCRIPTION: counts the number of elements in the endpoint receive queue
    identified by the receive handle.
  PARAMETERS: receive_handle - the receive channel handle
  RETURN VALUE: the number of elements in the receive queue
  ***************************************************************************/
  mcapi_uint_t mcapi_trans_sclchan_available_i( mcapi_sclchan_recv_hndl_t receive_handle)
  {
    uint16_t rd,rn,re;
    int rc = MCAPI_FALSE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_available_i(0x%x);",receive_handle);
    
    mcapi_assert(mcapi_trans_decode_handle(receive_handle,&rd,&rn,&re)); 
    rc = mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements;
    
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
    
    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_recv_close_i
  DESCRIPTION: non-blocking close of the receiving end of the scalar channel
  PARAMETERS: 
    receive_handle -
    request -
    mcapi_status -
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_sclchan_recv_close_i( mcapi_sclchan_recv_hndl_t  recv_handle,
                                         mcapi_request_t* request,
                                         mcapi_status_t* mcapi_status)
  {
    uint16_t rd,rn,re;
    int r;

    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE; 
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_recv_close_i(0x%x,&request,&status);",recv_handle);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(recv_handle,&rd,&rn,&re));
        mcapi_trans_close_channel_have_lock (rd,rn,re);
        completed = MCAPI_TRUE;    
      }  
      mcapi_assert(setup_request_have_lock(&recv_handle,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  
  /***************************************************************************
  NAME:mcapi_trans_sclchan_send_close_i
  DESCRIPTION: non-blocking close of the sending end of the scalar channel
  PARAMETERS: 
    send_handle -
    request -
    mcapi_status -
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_sclchan_send_close_i( mcapi_sclchan_send_hndl_t send_handle,
                                         mcapi_request_t* request,
                                         mcapi_status_t* mcapi_status)
  {
    uint16_t sd,sn,se;
    int r;
    
    /* if errors were found at the mcapi layer, then the request is considered complete */
    mcapi_boolean_t completed =  (*mcapi_status == MCAPI_SUCCESS) ? MCAPI_FALSE : MCAPI_TRUE;
    
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));
    
    mcapi_dprintf(1,"mcapi_trans_sclchan_send_close_i(0x%x,&request,&status);",send_handle);
    
    /* make sure we have a request entry */
    if ( mcapi_trans_reserve_request_have_lock(&r)) {
      if (!completed) {    
        mcapi_assert(mcapi_trans_decode_handle(send_handle,&sd,&sn,&se));
        mcapi_trans_close_channel_have_lock (sd,sn,se);
        completed = MCAPI_TRUE;
      }
      mcapi_assert(setup_request_have_lock(&send_handle,request,mcapi_status,completed,0,NULL,0,0,0,0,r));
    } else{
      *mcapi_status = MCAPI_ERR_REQUEST_LIMIT;
    }
    
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   test and wait functions                                //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  
  /***************************************************************************
  NAME:mcapi_trans_test_i
  DESCRIPTION: Tests if the request has completed yet (non-blocking).
        It is called mcapi_test at the mcapi level even though it's a non-blocking function.
  PARAMETERS: 
    request -
    size -
    mcapi_status -
  RETURN VALUE: TRUE/FALSE indicating if the request has completed.
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_test_i( mcapi_request_t* request, 
                                      size_t* size,
                                      mcapi_status_t* mcapi_status)  
  {
    
    /* We return true if it's cancelled, invalid or completed.  We only return
       false if the user should continue polling. 
    */
    mcapi_boolean_t rc = MCAPI_FALSE;
    uint16_t r;
    
    mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE);

    mcapi_dprintf(3,"mcapi_trans_test_i request handle:0x%lx",*request);
    
    if (!mcapi_trans_decode_request_handle(request,&r) || 
        (mcapi_db->requests[r].valid == MCAPI_FALSE)) {
      *mcapi_status = MCAPI_ERR_REQUEST_INVALID;
      rc = MCAPI_TRUE;
    } else if (mcapi_db->requests[r].cancelled) {
      *mcapi_status = MCAPI_ERR_REQUEST_CANCELLED;
      rc = MCAPI_TRUE;
    } else { 
      if (!(mcapi_db->requests[r].completed)) {
      /* try to complete the request */
      /*  receives to an empty channel or get_endpt for an endpt that
          doesn't yet exist are the only two types of non-blocking functions
          that don't complete immediately for this implementation */
      switch (mcapi_db->requests[r].type) {
      case (RECV) : 
        check_receive_request_have_lock (request); break;
      case (GET_ENDPT) :
        check_get_endpt_request_have_lock (request);break;
      case (OPEN_PKTCHAN) :
      case (OPEN_SCLCHAN) :
        check_open_channel_request_have_lock (request);
        break;
      default:
        mcapi_assert(0);
        break;
      };
    }
    
    /* query completed again because we may have just completed it */
    if (mcapi_db->requests[r].completed) {
      mcapi_trans_remove_request_have_lock(r);	/* by etem */
      *size = mcapi_db->requests[r].size;
      *mcapi_status = mcapi_db->requests[r].status;
      /* clear the entry so that it can be reused */
      memset(&mcapi_db->requests[r],0,sizeof(mcapi_request_data));
      *request=0;
      rc = MCAPI_TRUE;
    }
   }

    //mcapi_dprintf(2,"mcapi_trans_test_i returning rc=%u,status=%s",rc,mcapi_display_status(*mcapi_status));
    mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE);

    return rc;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_wait
  DESCRIPTION:Tests if the request has completed yet (non-blocking).
  PARAMETERS: 
    send_handle -
    request -
    mcapi_status -
  RETURN VALUE:  TRUE indicating the request has completed or FALSE
    indicating the request has been cancelled.
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_wait( mcapi_request_t* request, 
                                    size_t* size,
                                    mcapi_status_t* mcapi_status,
                                    mcapi_timeout_t timeout) 
  {
    mcapi_timeout_t time = 0;
    uint16_t r;
    mcapi_assert(mcapi_trans_decode_request_handle(request,&r));
    mcapi_dprintf(1,"mcapi_trans_wait(&request,&size,&status,%u);",timeout);
    while(1) {
      time++;
      if (mcapi_trans_test_i(request,size,mcapi_status)) {
        return MCAPI_TRUE;
      }
      /* yield */
      mcapi_dprintf(5,"mcapi_trans_wait - attempting to yield");
      /* we don't have the lock, it's safe to just yield */
      sched_yield();
      if ((timeout !=  MCA_INFINITE) && (time >= timeout)) {
        *mcapi_status = MCAPI_TIMEOUT;
        return MCAPI_FALSE;
      }
    }
  }
  
  /***************************************************************************
  NAME:mcapi_trans_wait_any
  DESCRIPTION:Tests if any of the requests have completed yet (blocking).
      Note: the request is now cleared if it has been completed or cancelled.
  PARAMETERS: 
    send_handle -
    request -
    mcapi_status -
  RETURN VALUE:
  ***************************************************************************/
  unsigned mcapi_trans_wait_any(size_t number, mcapi_request_t** requests, size_t* size,
                                       mcapi_status_t* mcapi_status,
                                       mcapi_timeout_t timeout) 
  {
    mcapi_timeout_t time = 0;
    int i;
    
    mcapi_dprintf(1,"mcapi_trans_wait_any");
    while(1) {
      time++;
      for (i = 0; i < number; i++) {
        if (mcapi_trans_test_i(requests[i],size,mcapi_status)) {
          return i;
        }
        /* yield */
        mcapi_dprintf(5,"mcapi_trans_wait_any - attempting to yield");
        /* we don't have the lock, it's safe to just yield */
        sched_yield();
        if ((timeout !=  MCA_INFINITE) && (time >= timeout)) {
          *mcapi_status = MCAPI_TIMEOUT;
          return MCA_RETURN_VALUE_INVALID;
        }
      }
    }
  }
  
  /***************************************************************************
  NAME:mcapi_trans_cancel
  DESCRIPTION: Cancels the given request
  PARAMETERS: 
    request -
    mcapi_status -
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_cancel(mcapi_request_t* request,mcapi_status_t* mcapi_status) 
  {
    uint16_t r;
    
    mcapi_dprintf(1,"mcapi_trans_cancel");
    
    /* lock the database */
    mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE);
    
    mcapi_assert(mcapi_trans_decode_request_handle(request,&r));
    
    if (mcapi_db->requests[r].valid == MCAPI_FALSE) {
      *mcapi_status = MCAPI_ERR_REQUEST_INVALID;
    } else if (mcapi_db->requests[r].cancelled) {
      /* this reqeust has already been cancelled */
      mcapi_dprintf(2,"mcapi_trans_cancel - request was already cancelled");
      *mcapi_status = MCAPI_ERR_REQUEST_CANCELLED;
    } else if (!(mcapi_db->requests[r].completed)) {
      /* cancel the request */
      mcapi_db->requests[r].cancelled = MCAPI_TRUE;
      switch (mcapi_db->requests[r].type) {
      case (RECV) : 
        cancel_receive_request_have_lock (request); break;
      case (GET_ENDPT) :
        break;
      default:
        mcapi_assert(0);
        break;
      };
      /* clear the request so that it can be re-used */
      memset(&mcapi_db->requests[r],0,sizeof(mcapi_request_data));
      *mcapi_status = MCAPI_SUCCESS;
      /* invalidate the request handle */
      //*request = 0;
    } else {
      /* it's too late, the request has already completed */
      mcapi_dprintf(2," mcapi_trans_cancel - Unable to cancel because request has already completed");
    }
    
    /* unlock the database */
    mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE);
  }
  
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   misc helper functions                                  //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  
  /***************************************************************************
  NAME:mcapi_trans_signal_handler 
  DESCRIPTION: The purpose of this function is to catch signals so that we
   can clean up our shared memory and sempaphore resources cleanly.
  PARAMETERS: the signal
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_signal_handler ( int sig ) 
  {
    
    uint32_t d,n;
    struct sigaction old_action, new_action; 
    mcapi_boolean_t last_man_standing = MCAPI_TRUE;
    
    mca_block_signals();
    
    /* try to lock the database */
    mcapi_boolean_t locked = mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE);
    
    /* print info on which signal was caught */
#ifdef __linux
    char sigbuff[128];
    sprintf(sigbuff,"SIGNAL: mcapi received signal[%d] pid=%d tid=%s dindex=%d nindex=%d mcapi_db=%p",
            sig,mcapi_pid,mca_print_tid(mcapi_tid),mcapi_dindex, mcapi_nindex,mcapi_db);
    psignal(sig,sigbuff); 
#else
    printf("mcapi received signal %s\n",strsignal(sig));
#endif
    
    if (mcapi_db) {    
      /* mark myself as invalid */
      mcapi_db->domains[mcapi_dindex].nodes[mcapi_nindex].valid = MCAPI_FALSE;
      
      /* mark any other nodes that belong to this process as invalid */
      for (d = 0; d < MCA_MAX_DOMAINS; d++) {
        for (n = 0; n < MCA_MAX_NODES; n++) {
          if ( mcapi_db->domains[d].nodes[n].valid) {
            if  ( mcapi_db->domains[d].nodes[n].pid == mcapi_pid) {
              mcapi_db->domains[d].nodes[n].valid = MCAPI_FALSE;
            } else {
              last_man_standing = MCAPI_FALSE;
            }
          }
        }
      }
      
      /* restore the old action */
      old_action = mcapi_db->domains[mcapi_dindex].nodes[mcapi_nindex].signals[sig];
      sigaction (sig, &old_action, NULL);
    } else {
      fprintf(stderr, "MRAPI: Unable to look up node/domain info for this process, thus unable to unwind the signal handlers any further. Restoring the default handler\n");
      new_action.sa_handler = SIG_DFL;
      sigemptyset (&new_action.sa_mask);
      new_action.sa_flags = 0;
      sigaction (sig, &new_action, NULL);
    }
    
    // unlock the database if we locked it
    if (locked) {
      mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE);
    }
    
    // clean up mcapi semaphore and shared memory
    transport_sm_finalize(last_man_standing,MCAPI_TRUE,MCAPI_FALSE,global_rwl);
    //mcapi_db = NULL;
    
    mca_unblock_signals();
    
    /* Now reraise the signal so that mrapi can do it's cleanup */
    raise (sig);
  }

  /***************************************************************************
  NAME: setup_request_have_lock
  DESCRIPTION: Sets up the request for a non-blocking function.
  PARAMETERS: 
     handle - 
     request -
     mcapi_status -
     completed - whether the request has already been completed or not (usually
       it has - receives to an empty queue or endpoint_get for endpoints that 
       don't yet exist are the two main cases where completed will be false)
     size - 
     buffer - the buffer
     type - the type of the request
  RETURN VALUE:
  ***************************************************************************/
  mcapi_boolean_t setup_request_have_lock (mcapi_endpoint_t* handle,
                                          mcapi_request_t* request,
                                          mcapi_status_t* mcapi_status, 
                                          mcapi_boolean_t completed, 
                                          size_t size,void** buffer,
                                          mcapi_request_type type,
                                          mcapi_uint_t node_num, 
                                          mcapi_uint_t port_num, 
                                          mcapi_domain_t domain_num,
                                          int r) 
  {
    int i,qindex;
    uint16_t d,n,e; 
    mcapi_boolean_t rc = MCAPI_TRUE;
    
    /* the database should already be locked */
    assert(locked == 1);

    mcapi_db->requests[r].status = *mcapi_status;
    mcapi_db->requests[r].size = size;
    mcapi_db->requests[r].cancelled = MCAPI_FALSE;
    mcapi_db->requests[r].completed = completed;
    
    //encode the request handle (this is the only place in the code we do this)
    *request = 0x80000000 | r;
    mcapi_dprintf(1,"setup_request_have_lock handle=0x%x",*request);  
    /* this is hacky, there's probably a better way to do this */
    if ((buffer != NULL) && (!completed)) {
      mcapi_assert(mcapi_trans_decode_handle(*handle,&d,&n,&e));
      if ( mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.channel_type == MCAPI_PKT_CHAN) {
        /* packet buffer means system buffer, so save the users pointer to the buffer */
        mcapi_db->requests[r].buffer_ptr = buffer;
      } else {
        /* message buffer means user buffer, so save the users buffer */
        mcapi_db->requests[r].buffer = *buffer;
      }
    }
    mcapi_db->requests[r].type = type;
    mcapi_db->requests[r].handle = *handle;
    
    /* save the pointer so that we can fill it in (the endpoint may not have been created yet) 
       an alternative is to make buffer a void* and use it for everything (messages, endpoints, etc.) */
    if (  mcapi_db->requests[r].type == GET_ENDPT) {
      mcapi_db->requests[r].ep_endpoint = handle; 
      mcapi_db->requests[r].ep_node_num = node_num;
      mcapi_db->requests[r].ep_port_num = port_num;
      mcapi_db->requests[r].ep_domain_num = domain_num;
    }
    
    /* if this was a non-blocking receive to an empty queue, then reserve the next buffer */
    if ((type == RECV) && (!completed)) {
      mcapi_assert(mcapi_trans_decode_handle(*handle,&d,&n,&e));
      /*find the queue entry that doesn't already have a request associated with it */
      for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
        /* walk from head to tail */
        qindex = (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.head + i) % (MCAPI_MAX_QUEUE_ENTRIES); 
        if ((mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.elements[qindex].request==0) && 
            (!mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.elements[qindex].invalid)) {
          mcapi_dprintf(4,"receive request r=%u reserving qindex=%i",i,qindex);
          mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.elements[qindex].request = *request; 
          break;
        }      
      }
      if (i == MCAPI_MAX_QUEUE_ENTRIES) {
        mcapi_dprintf(1,"setup_request_have_lock: MCAPI_ERR_MEM_LIMIT all of this endpoint's buffers already have requests associated with them.  Your receives are outpacing your sends.  Either throttle this at the application layer or reconfigure with a larger endpoint receive queue.");
        /* all of this endpoint's buffers already have requests associated with them */
        mcapi_db->requests[r].status = MCAPI_ERR_MEM_LIMIT;
        mcapi_db->requests[r].completed = MCAPI_TRUE;
      }   
    }
    
    return rc;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_decode_request_handle
  DESCRIPTION:
  PARAMETER:
  RETURN VALUE: 
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_decode_request_handle(mcapi_request_t* request,uint16_t* r) 
  {
    *r = *request;
    if ((*r < MCAPI_MAX_REQUESTS) && (*request & 0x80000000)) {
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_display_state
  DESCRIPTION: This function is useful for debugging.  If the handle is null,
   we'll print out the state of the entire database.  Otherwise, we'll print out
   only the state of the endpoint that the handle refers to.
  PARAMETERS: 
     handle
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_display_state (void* handle)
  {
    /* lock the database */
    mcapi_assert(mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE));  
    mcapi_trans_display_state_have_lock(handle);
    /* unlock the database */
    mcapi_assert(mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE));
  }
  
  /***************************************************************************
  NAME:mcapi_trans_display_state_have_lock
  DESCRIPTION: This function is useful for debugging.  If the handle is null,
   we'll print out the state of the entire database.  Otherwise, we'll print out
   only the state of the endpoint that the handle refers to.  Expects the database
   to be locked.
  PARAMETERS: 
     handle
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_display_state_have_lock (void* handle)
  {
    uint16_t d,n,e,a;
    mcapi_endpoint_t* endpoint = (mcapi_endpoint_t*)handle;
    
    printf("DISPLAY STATE:");
    
    
    if (handle != NULL) {
      /* print the data for the given endpoint */
      mcapi_assert(mcapi_trans_decode_handle(*endpoint,&d,&n,&e)); 
      printf("\nnode: %u, port: %u, receive queue (num_elements=%i):",
             (unsigned)mcapi_db->domains[d].nodes[n].node_num,(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num, 
             (unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue.num_elements);
      
      printf("\n    endpoint: %u",e);
      printf("\n      valid:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid);
      printf("\n      anonymous:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].anonymous);
      printf("\n      open:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
      printf("\n      connected:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected);
      printf("\n      num_attributes:%u",(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].num_attributes);
      for (a = 0; a < mcapi_db->domains[d].nodes[n].node_d.endpoints[e].num_attributes; a++) {
        printf("\n        attribute:%u",a);
        printf("\n          valid:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].valid);
        printf("\n          attribute_num:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].attribute_num);
        printf("\n          bytes:%i",(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].bytes);
      }
      //print_queue(mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue);
    } else {
      /* print the whole database */
      for (d = 0; d < MCA_MAX_DOMAINS; d++) {
        for (n = 0; n < MCA_MAX_NODES; n++) {
          if ((mcapi_db->domains[d].valid == MCAPI_TRUE) && (mcapi_db->domains[d].nodes[n].valid == MCAPI_TRUE)) {
            printf("\nVALID NODE: d=%u, nindex=%u domain_id=%u,node_num=%u,",
                   d,n,mcapi_db->domains[d].domain_id,(unsigned)mcapi_db->domains[d].nodes[n].node_num);
            printf("\n  num_endpoints:%u",mcapi_db->domains[d].nodes[n].node_d.num_endpoints);
            for (e = 0; e < mcapi_db->domains[d].nodes[n].node_d.num_endpoints; e++) {
              if (mcapi_db->domains[d].nodes[n].node_d.endpoints[e].valid) {
                printf("\n    VALID ENDPT: e=%u",e);
                printf("\n    port_num: %u",(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].port_num);
                printf("\n      anonymous:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].anonymous);
                printf("\n      open:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open);
                printf("\n      connected:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected);
                printf("\n      num_attributes:%u",(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].num_attributes);
                for (a = 0; a < mcapi_db->domains[d].nodes[n].node_d.endpoints[e].num_attributes; a++) {
                  printf("\n        a=%u",a);
                  printf("\n        attribute:%u",a);
                  printf("\n          valid:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].valid);
                  printf("\n          attribute_num:%u",mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].attribute_num);
                  printf("\n          bytes:%u",(unsigned)mcapi_db->domains[d].nodes[n].node_d.endpoints[e].attributes.entries[a].bytes);
                }
              }
              //print_queue(mcapi_db->domains[d].nodes[n].node_d.endpoints[e].recv_queue);
            }
          }
        }
      }
    }
    printf("\n ");
  }
  
  /***************************************************************************
  NAME:check_open_channel_request
  DESCRIPTION: Checks if the endpoint has been connected yet.
  PARAMETERS: the request pointer (to be filled in)
  RETURN VALUE: none
  ***************************************************************************/
  void check_open_channel_request_have_lock (mcapi_request_t *request) 
  {
    uint16_t d,n,e,r;
    assert(locked == 1);
    if (mcapi_trans_decode_request_handle(request,&r)) {
      
      mcapi_assert(mcapi_trans_decode_handle(mcapi_db->requests[r].handle,&d,&n,&e));
      
      /* has the channel been connected yet? */
      if ( mcapi_db->domains[d].nodes[n].node_d.endpoints[e].connected == MCAPI_TRUE) {
        mcapi_db->requests[r].completed = MCAPI_TRUE;
      }
    }
  } 
  
  /***************************************************************************
  NAME:check_get_endpt_request
  DESCRIPTION: Checks if the request to get an endpoint has been completed or not.
  PARAMETERS: the request pointer (to be filled in)
  RETURN VALUE: none
  ***************************************************************************/
  void check_get_endpt_request_have_lock (mcapi_request_t *request) 
  {
    
    uint16_t r;
    assert(locked == 1);
    mcapi_assert(mcapi_trans_decode_request_handle(request,&r));
    if (mcapi_trans_endpoint_get_have_lock (mcapi_db->requests[r].ep_endpoint, 
                                           mcapi_db->requests[r].ep_domain_num,
                                           mcapi_db->requests[r].ep_node_num, 
                                           mcapi_db->requests[r].ep_port_num)) {
      mcapi_db->requests[r].completed = MCAPI_TRUE;
      mcapi_db->requests[r].status = MCAPI_SUCCESS;
    }  
    
  }

  /***************************************************************************
  NAME: cancel_receive_request_have_lock
  DESCRIPTION: Cancels an outstanding receive request.  This is a little tricky
     because we have to preserve FIFO which means we have to shift all other
     outstanding receive requests down.
  PARAMETERS: 
     request -
  RETURN VALUE: none
  ***************************************************************************/
  void cancel_receive_request_have_lock (mcapi_request_t *request) 
  {
    uint16_t rd,rn,re,r;
    int i,last,start,curr;
    
    /* the database should already be locked */
    assert(locked == 1);
    mcapi_assert(mcapi_trans_decode_request_handle(request,&r));
    mcapi_assert(mcapi_trans_decode_handle(mcapi_db->requests[r].handle,&rd,&rn,&re));
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].request == *request) {
        /* we found the request, now clear the reservation */
        mcapi_dprintf(5,"cancel_receive_request - cancelling request at index %i BEFORE:",i);
        //print_queue(mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue);
        mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].request = 0;
        break;
      }
    }
    
    /* we should have found the outstanding request */
    mcapi_assert (i != MCAPI_MAX_QUEUE_ENTRIES);
    
    /* shift all pending reservations down*/
    start = i;
    last = start;
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      curr = (i+start)%MCAPI_MAX_QUEUE_ENTRIES;
      /* don't cross over the head or the tail */
      if ((curr == mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.tail) &&
          (curr != start)) {
        break;
      }
      if ((curr == mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.head) &&
          (curr != start)) {
        break;
      }
      if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[curr].request) {
        mcapi_dprintf(5,"cancel_receive_request - shifting request at index %i to index %i",curr,last);
        mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[last].request = 
          mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[curr].request;
        mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[curr].request = 0;
        last = curr;
      }
    }
    
    mcapi_db->requests[r].cancelled = MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME: check_receive_request
  DESCRIPTION: Checks if the given non-blocking receive request has completed.
     This is a little tricky because we can't just pop from the head of the
     endpoints receive queue.  We have to locate the reservation that was 
     made in the queue (to preserve FIFO) at the time the request was made.
  PARAMETERS: the request pointer (to be filled in
  RETURN VALUE: none
  ***************************************************************************/
  void check_receive_request_have_lock (mcapi_request_t *request) 
  {
    uint16_t rd,rn,re,r;
    int i;
    size_t size;
    
    /* the database should already be locked */
    assert (locked == 1);

    int32_t index=-1;
    mcapi_assert(mcapi_trans_decode_request_handle(request,&r));
    mcapi_assert(mcapi_trans_decode_handle(mcapi_db->requests[r].handle,&rd,&rn,&re));
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].request == *request) {
        /* we found the request, check to see if there is valid data in the receive queue entry */ 
        if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].buff_index ) {
          /* clear the request reservation */
          mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].request = 0;
          /* shared memory is zeroed, so we store our index as index+1 so that we can tell if it's valid or not*/
          index = mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].buff_index &~ MCAPI_VALID_MASK;
          /* update the request */
          mcapi_db->requests[r].completed = MCAPI_TRUE;
          mcapi_db->requests[r].status = MCAPI_SUCCESS;
          /* first take the entry out of the queue  this has the potential to fragment our
             receive queue since we may not be removing from the head */
          if ( mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_PKT_CHAN) {
            /* packet buffer means system buffer, so save the users pointer to the buffer */
            mcapi_trans_recv_have_lock_ (rd,rn,re,mcapi_db->requests[r].buffer_ptr,mcapi_db->requests[r].size,&mcapi_db->requests[r].size,i,NULL); 
          } else {
            /* message buffer means user buffer, so save the users buffer */
            size = mcapi_db->requests[r].size;
            mcapi_trans_recv_have_lock_ (rd,rn,re,&mcapi_db->requests[r].buffer,mcapi_db->requests[r].size,&mcapi_db->requests[r].size,i,NULL); 
            if (mcapi_db->requests[r].size > size) {
              mcapi_db->requests[r].size = size;
              mcapi_db->requests[r].status = MCAPI_ERR_MSG_TRUNCATED;
            } 
          }
          /* now update the receive queue state */
          mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements--;
          /* mark this entry as invalid so that the "bubble" won't be re-used */
          mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[i].invalid = MCAPI_TRUE;
          mcapi_trans_compact_queue (&mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue);
          mcapi_dprintf(4,"receive request (test/wait) popped from qindex=%i, num_elements=%i, head=%i, tail=%i",
                        i,mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements,
                        mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.head,
                        mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.tail);
        }
        break;
      }
    }
    /* we should have found the outstanding request */
    mcapi_assert (i != MCAPI_MAX_QUEUE_ENTRIES);    
  }
  
  /***************************************************************************
  NAME:mcapi_trans_connect_channel_have_lock
  DESCRIPTION: connects a channel
  PARAMETERS: 
     send_endpoint
     receive_endpoint
     type
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_connect_channel_have_lock (mcapi_endpoint_t send_endpoint,
                                             mcapi_endpoint_t receive_endpoint,
                                             channel_type type) 
  {
    uint16_t sd,sn,se;
    uint16_t rd,rn,re;
    
    /* the database should already be locked */
    assert(locked == 1);

    mcapi_assert(mcapi_trans_decode_handle(send_endpoint,&sd,&sn,&se));
    mcapi_assert(mcapi_trans_decode_handle(receive_endpoint,&rd,&rn,&re));
    
    /* update the send endpoint */
    mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].connected = MCAPI_TRUE;
    mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.recv_endpt = receive_endpoint;
    mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.send_endpt = send_endpoint; 
    mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].recv_queue.channel_type = type;
    
    /* update the receive endpoint */
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].connected = MCAPI_TRUE;
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.send_endpt = send_endpoint;
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.recv_endpt = receive_endpoint;
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type = type;
    
    
    mcapi_dprintf(1,"channel_type=%u connected sender (node=%u,port=%u) to receiver (node=%u,port=%u)", 
                  type,mcapi_db->domains[sd].nodes[sn].node_num,
                  mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].port_num,
                  mcapi_db->domains[rd].nodes[rn].node_num,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].port_num);
    
  }
  
  /***************************************************************************
  NAME:mcapi_trans_send_have_lock
  DESCRIPTION: Attempts to send a message from one endpoint to another
  PARAMETERS: 
    sn - the send node index (only used for verbose debug print)
    se - the send endpoint index (only used for verbose debug print)
    rn - the receive node index
    re - the receive endpoint index
    buffer -
    buffer_size -
  
  RETURN VALUE: true/false indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_send_have_lock (uint16_t sd, uint16_t sn,uint16_t se, 
                                             uint16_t rd,uint16_t rn, uint16_t re, 
                                             const char* buffer, 
                                             size_t buffer_size,
                                             uint64_t scalar)
  {
    int qindex,i;
    buffer_entry* db_buff = NULL;
    
    mcapi_dprintf(3,"mcapi_trans_send_have_lock sender (node=%u,port=%u) to receiver (node=%u,port=%u) ", 
                  mcapi_db->domains[sd].nodes[sn].node_num,
                  mcapi_db->domains[sd].nodes[sn].node_d.endpoints[se].port_num,
                  mcapi_db->domains[rd].nodes[rn].node_num,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].port_num);
    
    /* The database should already be locked! */
    assert(locked == 1);

    if (mcapi_trans_full_queue(mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue)) {
      /* we couldn't get space in the endpoints receive queue, try to compact the queue */
      mcapi_trans_compact_queue(&mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue);
      return MCAPI_FALSE;
    }
    
    /* find a free mcapi buffer (we only have to worry about this on the sending side) */
    for (i = 0; i < MCAPI_MAX_BUFFERS; i++) {
      if (!mcapi_db->buffers[i].magic_num) {
        mcapi_db->buffers[i].magic_num = MAGIC_NUM;
        db_buff = &mcapi_db->buffers[i];
        mcapi_dprintf(4,"using buffer index i=%u\n",i);
        break;
      }
    }
    if (i == MCAPI_MAX_BUFFERS) {
      /* we couldn't get a free buffer */
      mcapi_dprintf(2,"ERROR mcapi_trans_send_have_lock: No more buffers available - try freeing some buffers. ");
      return MCAPI_FALSE;
    }
    
    /* now go about updating buffer into the database... */
    /* find the next index in the circular queue */
    qindex = mcapi_trans_push_queue(&mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue);
    mcapi_dprintf(4,"send pushing %u byte buffer to qindex=%i, num_elements=%i, head=%i, tail=%i",
                  buffer_size,qindex,mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.head,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.tail);
    /* printf(" send pushing to qindex=%i",qindex); */ 
    if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_SCL_CHAN ) {
      db_buff->scalar = scalar;
    } else {
      /* copy the buffer parm into a mcapi buffer */
      memcpy (db_buff->buff,buffer,buffer_size);
    }
    /* set the size */
    db_buff->size = buffer_size;
    /* update the ptr in the receive_endpoints queue to point to our mcapi buffer */
    /* shared memory is zeroed, so we store our index as index with a valid bit so that we can tell if it's valid or not*/
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[qindex].buff_index = i | MCAPI_VALID_MASK;
    
    
    return MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME:  mcapi_trans_recv_have_lock_
  DESCRIPTION: Removes a message (at the given qindex) from the given 
    receive endpoints queue.  This function is used both by check_receive_request
    and mcapi_trans_recv_have_lock.  We needed to separate the functionality
    because in order to preserve FIFO, if recv was called to an empty queue we
    had to set a reservation at the head of the queue.  Thus we can't always
    just pop from the head of the queue.
  PARAMETERS: 
    rn - the receive node index
    re - the receive endpoint index
    buffer -
    buffer_size -
    received_size - the actual size (in bytes) of the data received
    qindex - index into the receive endpoints queue that we should remove from
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_recv_have_lock_ (uint16_t rd,uint16_t rn, uint16_t re, void** buffer, size_t buffer_size,
                                   size_t* received_size,int qindex,uint64_t* scalar)
  {
    size_t size;
    
    
    /* the database should already be locked! */
    assert(locked == 1);

    /* shared memory is zeroed, so we store our index as index w/ a valid bit so that we can tell if it's valid or not*/
    int index = mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[qindex].buff_index &~ MCAPI_VALID_MASK;
    mcapi_assert (index >= 0);
    
    mcapi_dprintf(3,"mcapi_trans_recv_have_lock_ for receiver (node=%u,port=%u)", 
                  mcapi_db->domains[rd].nodes[rn].node_num,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].port_num);
    
    /* printf(" recv popping from qindex=%i",qindex); */
    /* first make sure buffer is big enough for the message */
    if ((buffer_size) < mcapi_db->buffers[index].size) {
      fprintf(stderr,"ERROR: mcapi_trans_recv_have_lock buffer not big enough - loss of data: buffer_size=%i, element_size=%i",
              (int)buffer_size,
              (int)mcapi_db->buffers[index].size);
      /* NOTE: MCAPI_ETRUNCATED will be set by the calling functions by noticing that buffer_size < received_size */
    }
    
    /* set the size */
    size = mcapi_db->buffers[index].size;
    
    /* fill in the size */
    *received_size = size;
    if (buffer_size < size) {
      size = buffer_size;
    } 
    
    
    /* copy the buffer out of the receive_endpoint's queue and into the buffer parm */
    if (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_PKT_CHAN) {
      /* mcapi supplied buffer (pkt receive), so just update the pointer */
      *buffer = mcapi_db->buffers[index].buff;
    } else {
      /* user supplied buffer, copy it in and free the mcapi buffer */
      if   (mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.channel_type == MCAPI_SCL_CHAN) {
        /* scalar receive */
        *scalar = mcapi_db->buffers[index].scalar;
      } else {
        /* msg receive */
        memcpy (*buffer,mcapi_db->buffers[index].buff,size);
      }   
      /* free the mcapi  buffer */
      memset(&mcapi_db->buffers[index],0,sizeof(mcapi_db->buffers[index]));
    }
    mcapi_dprintf(4,"receive popping %u byte buffer from qindex=%i, num_elements=%i, head=%i, tail=%i buffer=[",
                  size,qindex,mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.num_elements,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.head,
                  mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.tail);
    
    /* clear the buffer pointer in the receive queue entry */
    mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue.elements[qindex].buff_index = 0;
    
  }
  
  /***************************************************************************
  NAME: mcapi_trans_recv_have_lock
  DESCRIPTION: checks if a message is available, if so performs the pop (from
   the head of the queue) and sends the qindex to be used to mcapi_trans_recv_have_lock_ 
  PARAMETERS: 
    rn - the receive node index
    re - the receive endpoint index
    buffer -
    buffer_size -
    received_size - the actual size (in bytes) of the data received
    blocking - whether or not this is a blocking receive
  RETURN VALUE: true/false indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_recv_have_lock (uint16_t rd,uint16_t rn, uint16_t re, void** buffer, 
                                             size_t buffer_size, size_t* received_size,
                                             mcapi_boolean_t blocking,uint64_t* scalar)
  {
    int qindex;
    
    /* The database should already be locked! */
    assert(locked == 1);

    if ((!blocking) && (mcapi_trans_empty_queue(mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue))) {
      return MCAPI_FALSE;
    } 
    
    while (mcapi_trans_empty_queue(mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue)) {
      mcapi_dprintf(5,"mcapi_trans_recv_have_lock to empty queue - attempting to yield");
      /* we have the lock, use this yield */
      mcapi_trans_yield_have_lock();
    }
    
    /* remove the element from the receive endpoints queue */
    qindex = mcapi_trans_pop_queue(&mcapi_db->domains[rd].nodes[rn].node_d.endpoints[re].recv_queue);
    mcapi_trans_recv_have_lock_ (rd,rn,re,buffer,buffer_size,received_size,qindex,scalar);
    
    return MCAPI_TRUE;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_open_channel_have_lock
  DESCRIPTION: marks the given endpoint as open
  PARAMETERS: 
    n - the node index
    e - the endpoint index
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_open_channel_have_lock (uint16_t d,uint16_t n, uint16_t e) 
  {
    
    /* The database should already be locked! */
    assert(locked == 1);

    /* mark the endpoint as open */
    mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open = MCAPI_TRUE;
    
  }
  
  /***************************************************************************
  NAME:mcapi_trans_close_channel_have_lock
  DESCRIPTION: marks the given endpoint as closed
  PARAMETERS: 
    n - the node index
    e - the endpoint index
  RETURN VALUE:none
  ***************************************************************************/
  void mcapi_trans_close_channel_have_lock (uint16_t d,uint16_t n, uint16_t e) 
  {
    
    /* The database should already be locked! */
    assert(locked == 1);

    /* mark the endpoint as closed */
    mcapi_db->domains[d].nodes[n].node_d.endpoints[e].open = MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_yield_have_lock
  DESCRIPTION: releases the lock, attempts to yield, re-acquires the lock.
  PARAMETERS: none
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_yield_have_lock () 
  {  
    /* call this version of sched_yield when you have the lock */
    assert(locked == 1);

    /* release the lock */
    mcapi_trans_access_database_post(global_rwl,MCAPI_TRUE);
    assert(locked == 0);
    sched_yield();
    /* re-acquire the lock */
    mcapi_trans_access_database_pre(global_rwl,MCAPI_TRUE);
    assert (locked == 1);
  }
  
  /***************************************************************************
  NAME: mcapi_trans_access_database_pre
  DESCRIPTION: This function acquires the semaphore.
  PARAMETERS: none
  RETURN VALUE:none
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_access_database_pre (uint32_t handle,
                                                   mcapi_boolean_t exclusive) 
  {
    
    /* first acquire the semaphore, this is a blocking function */
    assert(locked == 0);
    if (transport_sm_lock_rwl(handle,exclusive)) {
      locked++;
      mcapi_dprintf(4,"mcapi_trans_access_database_pre ()");
      return MCAPI_TRUE;
    } 
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_access_database_post
  DESCRIPTION: This function releases the semaphore.
  PARAMETERS: none
  RETURN VALUE: none
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_access_database_post (uint32_t handle,
                                                    mcapi_boolean_t exclusive) 
  {
    assert(locked == 1);
    mcapi_dprintf(4,"mcapi_trans_access_database_post ()");
    
    /* finally, release the semaphore, this should always work */
    if (transport_sm_unlock_rwl(handle,exclusive)) {
      locked--;
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_encode_handle 
  DESCRIPTION:
   Our handles are very simple - a 32 bit integer is encoded with 
   an index (16 bits gives us a range of 0:64K indices).
   Currently, we only have 3 indices for each of: domain array,
   node array, and endpoint array.
  PARAMETERS: 
   node_index -
   endpoint_index -
  RETURN VALUE: the handle
  ***************************************************************************/
  uint32_t mcapi_trans_encode_handle (uint16_t domain_index,uint16_t node_index,uint16_t endpoint_index) 
  {
    /* The database should already be locked */
    uint32_t handle = 0;
    uint8_t shift = 8;
    
    mcapi_assert ((domain_index < MCA_MAX_DOMAINS) && 
            (node_index < MCA_MAX_NODES) && 
            (endpoint_index < MCAPI_MAX_ENDPOINTS));
    
    handle = domain_index;
    handle <<= shift;
    handle |= node_index;
    handle <<= shift;
    handle |= endpoint_index;
    
    return handle;
  }
  
  /***************************************************************************
  NAME:mcapi_trans_decode_handle
  DESCRIPTION: Decodes the given handle into it's database indices
  PARAMETERS: 
   handle -
   node_index -
   endpoint_index -
  RETURN VALUE: true/false indicating success or failure
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_decode_handle (uint32_t handle, uint16_t *domain_index,uint16_t *node_index,
                                                      uint16_t *endpoint_index) 
  {
    int rc = MCAPI_FALSE;
    uint8_t shift = 8;
    
    /* The database should already be locked */
    *domain_index            = (handle & 0x00ff0000) >> (shift * 2);
    *node_index              = (handle & 0x0000ff00) >> shift;
    *endpoint_index          = (handle & 0x000000ff);
    
    if ((*domain_index < MCA_MAX_DOMAINS) && 
        (*node_index < MCA_MAX_NODES) && 
        (*endpoint_index < MCAPI_MAX_ENDPOINTS)) {
      rc = MCAPI_TRUE;
    }
    
    return rc;
  }
  
  
  /***************************************************************************
  Function: mcapi_trans_whoami

  Description: Gets the pid,tid pair for the caller and  then
      looks up the corresponding node and domain info in our database.

  Parameters:

  Returns: boolean indicating success or failure

  ***************************************************************************/
  inline mcapi_boolean_t mcapi_trans_whoami (mcapi_node_t* node_id,uint32_t* n_index,
                                                      mcapi_domain_t* domain_id,uint32_t* d_index)
  {
    
    if (mcapi_db == NULL) { return MCAPI_FALSE;}
    *n_index = mcapi_nindex;
    *d_index = mcapi_dindex;
    *node_id = mcapi_node_num;
    *domain_id = mcapi_domain_id;
    return MCAPI_TRUE;
  }
  
  
  
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  //                   queue management                                       //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  /*
    
  This is my least favorite section of code in our MRAPI implementation.  I'm sure
  there are queue libraries out there that are prettier than this but we can't use
  them.  Here's why.  Conceptually you can think of each endpoint as having a 
  receive queue that messages/packets/scalars get put into.  The problem is that
  it's not really a queue because of complications due to handling non-blocking
  semantics.
  1) requests are cancellable, this leaves bubbles in the queue
  2) requests are satisfied in a fifo order however we have no idea when the
  user will call test/wait/cancel.  When the user calls test/wait/cancel we
  access the element directly (no longer fifo).
  */
  /***************************************************************************
  NAME: print_queue
  DESCRIPTION: Prints an endpoints receive queue (useful for debugging)
  PARAMETERS: q - the queue
  RETURN VALUE: none
  ***************************************************************************/
  void print_queue (queue q) 
  {
    int i,qindex,index;
    uint16_t r;
    /*print the recv queue from head to tail*/
    printf("\n      recv_queue:");
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      /* walk the queue from the head to the tail */
      qindex = (q.head + i) % (MCAPI_MAX_QUEUE_ENTRIES);   
      printf("\n          ----------------QINDEX: %i",qindex);
      if (q.head == qindex) { printf("\n           *** HEAD ***"); }
      if (q.tail == qindex) { printf("\n           *** TAIL ***"); }
      printf("\n          request:0x%lx",(long unsigned int)q.elements[qindex].request);
      if (q.elements[qindex].request) {
        r = q.elements[qindex].request;
        printf("\n             valid:%u",mcapi_db->requests[r].valid);
        printf("\n             size:%u",(int)mcapi_db->requests[r].size);
        switch (mcapi_db->requests[r].type) {
        case (OTHER): printf("\n             type:OTHER"); break;
        case (SEND): printf("\n             type:SEND"); break;
        case (RECV): printf("\n             type:RECV"); break;
        case (GET_ENDPT): printf("\n             type:GET_ENDPT"); break;
        default:  printf("\n             type:UNKNOWN!!!"); break;
        };
        printf("\n             buffer:[%s]",(char*)mcapi_db->requests[r].buffer);
        printf("\n             buffer_ptr:0x%lx",(long unsigned int)mcapi_db->requests[r].buffer_ptr);
        printf("\n             completed:%u",mcapi_db->requests[r].completed);
        printf("\n             cancelled:%u",mcapi_db->requests[r].cancelled);
        printf("\n             handle:0x%i",(int)mcapi_db->requests[r].handle);
        /*   printf("\n             status:%s",mcapi_display_status(mcapi_db->requests[r].status)); */
        printf("\n             status:%i",(int)mcapi_db->requests[r].status);
        printf("\n             endpoint:0x%lx",(long unsigned int)mcapi_db->requests[r].ep_endpoint);
      }
      printf("\n          invalid:%u",q.elements[qindex].invalid);
      
      printf("\n          b:0x%lx",(long unsigned int)q.elements[qindex].buff_index);
      if (q.elements[qindex].buff_index) {
        index = q.elements[qindex].buff_index &~ MCAPI_VALID_MASK;
        printf("\n             size:%u",(unsigned)mcapi_db->buffers[index].size);
        printf("\n             magic_num:%x",(unsigned)mcapi_db->buffers[index].magic_num);
        printf("\n             buff:[%s]",(char*)mcapi_db->buffers[index].buff);
      }
    }   
  }
  
  /***************************************************************************
  NAME: push_queue
  DESCRIPTION: Returns the qindex that should be used for adding an element.
     Also updates the num_elements, and tail pointer.
  PARAMETERS: q - the queue pointer
  RETURN VALUE: the qindex to be used
  ***************************************************************************/
  int mcapi_trans_push_queue(queue* q) 
  {
    int i;
    /* the database should be locked */
    assert(locked == 1);
    if ( (q->tail + 1) % MCAPI_MAX_QUEUE_ENTRIES == q->head) {
      /* mcapi_assert (q->num_elements ==  MCAPI_MAX_QUEUE_ENTRIES);*/
      mcapi_assert(!"push_queue called on full queue");
    }
    q->num_elements++;
    i = q->tail;
    q->tail = ++q->tail % MCAPI_MAX_QUEUE_ENTRIES;
    mcapi_assert (q->head != q->tail);
    return i;
  }
  
  /***************************************************************************
  NAME: pop_queue
  DESCRIPTION: Returns the qindex that should be used for removing an element.
     Also updates the num_elements, and head pointer.  
  PARAMETERS: q - the queue pointer
  RETURN VALUE: the qindex to be used
  ***************************************************************************/
  int mcapi_trans_pop_queue (queue* q) 
  {
    int i,qindex;
    int x = 0;
    /* the database should be locked */
    assert(locked == 1);
    if (q->head == q->tail) {
      /*mcapi_assert (q->num_elements ==  0);*/
      mcapi_assert (!"pop_queue called on empty queue");
    }
    
    /* we can't just pop the first element off the head of the queue, because it
       may be reserved for an earlier recv call, we need to take the first element
       that doesn't already have a request associated with it.  This can fragment
       our queue. */
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      /* walk the queue from the head to the tail */
      qindex = (q->head + i) % (MCAPI_MAX_QUEUE_ENTRIES); 
      if ((!q->elements[qindex].request) &&
          (q->elements[qindex].buff_index)){   
        x = qindex;
        break;
      }      
    }
    if (i == MCAPI_MAX_QUEUE_ENTRIES) {
      /* all of this endpoint's buffers already have requests associated with them */
      mcapi_assert(0); /* mcapi_trans_empty_queue should have already checked for this case */
    }
    
    q->num_elements--;
    
    /* if we are removing from the front of the queue, then move head */
    if (x == q->head) {
      q->head = ++q->head % MCAPI_MAX_QUEUE_ENTRIES; 
    } else {
      /* we are fragmenting the queue, mark this entry as invalid */
      q->elements[qindex].invalid = MCAPI_TRUE;
    }
    
    if (q->num_elements > 0) {
      if (q->head == q->tail) { printf("num_elements=%d\n",q->num_elements); }
      mcapi_assert (q->head != q->tail);
    }
    
    mcapi_trans_compact_queue (q);
    
    return x;
  }
  
  /***************************************************************************
  NAME: compact_queue
  DESCRIPTION: Attempts to compact the queue.  It can become fragmented based 
     on the order that blocking/non-blocking sends/receives/tests come in
  PARAMETERS: q - the queue pointer
  RETURN VALUE: none
  ***************************************************************************/
  void mcapi_trans_compact_queue (queue* q) 
  {
    int i;
    int qindex;
    /* the database should be locked */
    assert(locked == 1);
    mcapi_dprintf(7,"before mcapi_trans_compact_queue head=%i,tail=%i,num_elements=%i",q->head,q->tail,q->num_elements);
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
    qindex = (q->head + i) % (MCAPI_MAX_QUEUE_ENTRIES); 
    if ((qindex == q->tail) || 
        (q->elements[qindex].request) || 
        (q->elements[qindex].buff_index )){ 
      break;
    } else {
      /* advance the head pointer */ 
      q->elements[qindex].invalid = MCAPI_FALSE;
      q->head = ++q->head % MCAPI_MAX_QUEUE_ENTRIES; 
      i--;
    } 
    }
    mcapi_dprintf(7,"after mcapi_trans_compact_queue head=%i,tail=%i,num_elements=%i",q->head,q->tail,q->num_elements);
    if (q->num_elements > 0) {
      mcapi_assert (q->head != q->tail);
    }
    
  }
  
  /***************************************************************************
  NAME: mcapi_trans_empty_queue
  DESCRIPTION: Checks if the queue is empty or not
  PARAMETERS: q - the queue 
  RETURN VALUE: true/false
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_empty_queue (queue q) 
  {
    int i,qindex;
    /* the database should be locked */
    assert(locked == 1);

    if  (q.head == q.tail) {
      /* mcapi_assert (q.num_elements ==  0); */
      return MCAPI_TRUE;
    }
    
    /* if we have any buffers in our queue that don't have
       reservations, then our queue is non-empty */
    for (i = 0; i < MCAPI_MAX_QUEUE_ENTRIES; i++) {
      qindex = (q.head + i) % (MCAPI_MAX_QUEUE_ENTRIES); 
      if ((!q.elements[qindex].request) && 
          (q.elements[qindex].buff_index )){ 
        break;
      }
    }
    if (i == MCAPI_MAX_QUEUE_ENTRIES) {
      return MCAPI_TRUE;
    }
    
    return MCAPI_FALSE;
  }
  
  /***************************************************************************
  NAME: mcapi_trans_full_queue
  DESCRIPTION: Checks if the queue is full or not
  PARAMETERS: q - the queue 
  RETURN VALUE: true/false
  ***************************************************************************/
  mcapi_boolean_t mcapi_trans_full_queue (queue q) 
  {  
    /* the database should be locked */
    assert(locked == 1);
    if ( (q.tail + 1) % MCAPI_MAX_QUEUE_ENTRIES == q.head) {
      /*  mcapi_assert (q.num_elements ==  (MCAPI_MAX_QUEUE_ENTRIES -1)); */
      return MCAPI_TRUE;
    }
    return MCAPI_FALSE;
  }
  
  
#ifdef __cplusplus
  extern } 
#endif /* __cplusplus */
