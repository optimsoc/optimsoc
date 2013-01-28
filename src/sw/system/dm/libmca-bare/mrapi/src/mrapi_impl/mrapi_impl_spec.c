/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the followi]ng disclaimer.
 
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


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
                                                    
#include <mca_utils.h>
#include <mrapi_sys.h>
#include "resource_tree.h"

#include <string.h>
  
  /* do not put the following globals in thread-local-storage for two reasons:
     1) they are reset when a system call occurs which prevents us from being able
     to clean up properly
     2) gdb can't see thread local storage and since the database is one of the main */
  mrapi_database* mrapi_db; /* our shared memory addr for our internal database */
  int shmemid = -1; /* our shared memory id for our internal database */
  int semid = -1; /* our global semaphore id for locking/unlocking our database */

#define TLS __thread  
  TLS mrapi_resource_t* resource_root; /* root of the resource tree */
  TLS struct sigaction alarm_struct; /* used for testing resource tree */
  TLS pid_t mrapi_pid = -1;
  TLS pthread_t mrapi_tid;
  TLS unsigned mrapi_nindex;
  TLS unsigned mrapi_dindex;
  TLS mrapi_node_t mrapi_node_id;
  TLS mrapi_domain_t mrapi_domain_id;
  // finer grained locks for these sections of the database
  TLS int requests_semid; // requests array
  TLS int sems_semid;   // sems array
  TLS int shmems_semid; // shmems array
  TLS int rmems_semid;  // rmems array

  // tell the system whether or not to use the finer-grained locking
#define use_global_only 0

/*-------------------------------------------------------------------
  the mrapi_impl private function declarations (not part of the API)
  -------------------------------------------------------------------*/
/* Note:
  Anytime we read or write the database, we must be in a critical section.  Thus
  you will see a call to access_database_pre(acquires the semaphore) at the beginning
  of most functions and a call to access_database_post (releases the semaphore) at the
  end of most functions.  If a function has _locked appended to it's name, then
  it expects the semaphore to already be locked. */

/* mutexes, semaphores and reader-writer locks share a lot of the same code in this implementation */
  int32_t mrapi_impl_acquire_lock_locked(mrapi_sem_hndl_t sem, 
                                        int32_t num_locks, 
                                        mrapi_status_t* status);
  
  mrapi_boolean_t mrapi_impl_release_lock(mrapi_sem_hndl_t sem,
                                         int32_t num_locks, 
                                         mrapi_status_t* mrapi_status);
  
  mrapi_boolean_t mrapi_impl_create_lock_locked(mrapi_sem_hndl_t* sem,  
                                                  mrapi_sem_id_t key,
                                                  mrapi_uint32_t shared_lock_limit,
                                                  lock_type t,
                                                  mrapi_status_t* mrapi_status);
  
  mrapi_boolean_t mrapi_impl_valid_lock_hndl (mrapi_sem_hndl_t sem,
                                              mrapi_status_t* status);
  
  mrapi_boolean_t mrapi_impl_access_database_pre (int id,
                                                  int member,
                                                  mrapi_boolean_t fail_on_error);
  
  mrapi_boolean_t mrapi_impl_access_database_post (int id,int member);
  
  mrapi_boolean_t mrapi_impl_whoami (mrapi_node_t* node_id,
                                              uint32_t* n_index,
                                              mrapi_domain_t* domain_id,
                                              uint32_t* d_index);
  
  mrapi_boolean_t mrapi_impl_free_resources(mrapi_boolean_t panic);
  
  uint32_t mrapi_impl_encode_hndl (uint16_t type_index);
  
  mrapi_boolean_t mrapi_impl_decode_hndl (uint32_t handle,uint16_t *type_index);
  
  /* resource utilities */
  uint16_t mrapi_impl_get_number_of_nodes(mrapi_resource_type rsrc_type,
                                          mrapi_resource_t *tree);
  
  uint16_t mrapi_impl_create_rsrc_tree(mrapi_resource_type rsrc_type,
                                       mrapi_resource_t *src_tree,
                                       uint16_t start_index_child,
                                       mrapi_resource_t *filtered_tree);
  
  void mrapi_impl_increment_cache_hits(mrapi_resource_t *resource, 
                                       int increment);
  
  void mrapi_impl_trigger_rollover(uint16_t index);
  
  void mrapi_impl_cause_event();
  
  void mrapi_impl_alarm_catcher(int signal);
  
#define mrapi_assert(x) MRAPI_ASSERT(x,__LINE__);
  void MRAPI_ASSERT(mrapi_boolean_t condition,unsigned line) {
    if (!condition) {
      fprintf(stderr,"INTERNAL ERROR: MRAPI failed assertion (mrapi_impl_spec.c:%d) shutting down\n",line);
      mrapi_impl_free_resources(1/*panic*/);
      exit(1);
    }
  }
  
  /*-------------------------------------------------------------------
    the mrapi_impl API function definitions
    -------------------------------------------------------------------*/
  
  /***************************************************************************
  Function:  mrapi_impl_access_database_pre
    
  Description: locks the database (blocking)
  
  Parameters: none
  
  Returns: none
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_access_database_pre (int id,int member,mrapi_boolean_t fail_on_error) 
  {
    // if we are told to only use the global sem, then ignore the id passed in
    if (use_global_only) {
      id = semid;
      member = 0;
    }
    if (!sys_sem_lock(id,member)) {
      mrapi_dprintf(4,"mrapi_impl_access_database_pre - errno:%s",strerror(errno));
      //fprintf(stderr,"FATAL ERROR: unable to lock mrapi database: errno:%s\n",strerror(errno));
      if (fail_on_error) {
        fprintf(stderr,"FATAL ERROR: unable to lock mrapi database: errno:%s id=%x\n",strerror(errno),id);
        exit(1);
      }
      return MRAPI_FALSE;
    }
    
    mrapi_dprintf(4,"mrapi_impl_access_database_pre (got the internal mrapi db lock)");
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function:  mrapi_impl_access_database_post 
  
  Description: unlocks the database
  
  Parameters: none
  
  Returns: none
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_access_database_post (int id,int member) 
  {
    // if we are told to only use the global sem, then ignore the id passed in
    if (use_global_only) {
      id = semid;
      member = 0;
    }
    mrapi_dprintf(4,"mrapi_impl_access_database_post (released the internal mrapi db lock)");
    if (!sys_sem_unlock(id,member)) {
      mrapi_dprintf(4,"mrapi_impl_access_database_post (id=%d)- errno:%s",id,strerror(errno));
      fflush(stdout);
      return MRAPI_FALSE;
    }
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_whoami
  
  Description: Gets the pid,tid pair for the caller and  then
      looks up the corresponding node and domain info in our database.
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  inline mrapi_boolean_t mrapi_impl_whoami (mrapi_node_t* node_id,
                                                     uint32_t* n_index,
                                                     mrapi_domain_t* domain_id,
                                                     uint32_t* d_index)
  {
    
    if (mrapi_db == NULL) { return MRAPI_FALSE;}
    if (mrapi_pid == -1)  { return MRAPI_FALSE;}
    *n_index = mrapi_nindex;
    *d_index = mrapi_dindex;
    *node_id = mrapi_node_id;
    *domain_id = mrapi_domain_id;
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_encode_hndl
  Description:
   Our handles are very simple - a 32 bit integer is encoded with
   an index (16 bits gives us a range of 0:64K indices).  The index is the
   index into the sems, shmems or rmems array depending on what type of
   resource we are dealing with.

  Parameters:
  
  Returns: the handle
  ***************************************************************************/
  uint32_t mrapi_impl_encode_hndl (uint16_t type_index)
  {
    uint32_t valid = 0x80000000;
    return (valid | type_index);
  }
  
  /***************************************************************************
  Function: mrapi_impl_decode_hndl

  Description: Decodes the given handle into it's database index.

  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_decode_hndl (uint32_t handle,
                                          uint16_t *type_index)
  {
    uint32_t valid = 0x80000000;
    
    *type_index = (handle & 0x000000ff);
    
    /* check that the valid bit is set */
    if ((handle & valid) == valid) {
      return MRAPI_TRUE;
    }
    return MRAPI_FALSE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_test
  Description: Checks if the request has completed 
  Parameters:

  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_test(const mrapi_request_t* request,
                                  mrapi_status_t* status)
  {
    mrapi_boolean_t rc = MRAPI_FALSE;
    uint16_t r;
    
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(requests_semid,0,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_test r=%08x",*request);
    
    if (mrapi_impl_decode_hndl(*request,&r) && 
        (r<MRAPI_MAX_REQUESTS) && 
        (mrapi_db->requests[r].valid==MRAPI_TRUE)) {
      if (mrapi_db->requests[r].completed) {
        *status = mrapi_db->requests[r].status;
        rc = MRAPI_TRUE;
        //free up the request
        memset(&mrapi_db->requests[r],0,sizeof(mrapi_request_data)); 
      }
    }
    
    /* mrapi non-blocking functions always complete immediately */
    if (!rc) {
      mrapi_dprintf(1,"ASSERT: request=%08x r=%d valid=%d completed=%d\n",
                    *request,r,mrapi_db->requests[r].valid,mrapi_db->requests[r].completed);
      mrapi_dprintf(1,"  If you are seeing this error, it's possible you have already called test/wait on this request (we recycle the requests).");
    }
    mrapi_assert(rc);
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(requests_semid,0));
    
    return rc;
  } 
  
  /***************************************************************************
  Function: mrapi_impl_canceled_request
  Description: Checks if the request handle refers to a canceled request
  Parameters:

  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_canceled_request (const mrapi_request_t* request){
    mrapi_boolean_t rc = MRAPI_FALSE;
    uint16_t r;
    
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(requests_semid,0,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_canceled_request");
    
    if (mrapi_impl_decode_hndl(*request,&r) && 
        (r < MRAPI_MAX_REQUESTS) && 
        (mrapi_db->requests[r].valid == MRAPI_TRUE)) {
      rc = mrapi_db->requests[r].cancelled;
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(requests_semid,0));
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_setup_request
  Description: 
  Parameters:

  Returns: the index into the requests array
  ***************************************************************************/
  unsigned mrapi_impl_setup_request() {
    mrapi_node_t node;
    mrapi_domain_t domain;
    uint32_t n = 0;
    uint32_t d = 0;
    uint32_t i = 0;
    mrapi_boolean_t rc;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(requests_semid,0,MRAPI_TRUE));
    
    rc = mrapi_impl_whoami(&node,&n,&domain,&d);
    
    if (rc) {
      for (i = 0; i < MRAPI_MAX_REQUESTS; i++) {
        if (mrapi_db->requests[i].valid == MRAPI_FALSE) {
          mrapi_db->requests[i].valid = MRAPI_TRUE;
          mrapi_db->requests[i].domain_id = domain;
          mrapi_db->requests[i].node_num = node;
          mrapi_db->requests[i].cancelled = MRAPI_FALSE;
          break;
        }
      }
    }
    
    if (i < MRAPI_MAX_REQUESTS ) { 
      mrapi_dprintf(3,"mrapi_impl_setup_request i=%d valid=%d domain_id=%d node_num=%d cancelled = %d",
                    i,
                    mrapi_db->requests[i].valid,
                    mrapi_db->requests[i].domain_id,
                    mrapi_db->requests[i].node_num,
                    mrapi_db->requests[i].cancelled);	
    } else {
      mrapi_assert (0);
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(requests_semid,0));
    return i;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_request_hndl
  Description: Checks if the request handle refers to a valid request
  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_request_hndl (const mrapi_request_t* request) 
  {
    uint16_t r;
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(requests_semid,0,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_valid_request_handle handle=0x%x",*request);
    
    if (mrapi_impl_decode_hndl(*request,&r) && 
        (r < MRAPI_MAX_REQUESTS) && 
        (mrapi_db->requests[r].valid==MRAPI_TRUE)) {
      rc = MRAPI_TRUE;
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(requests_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_mutex_hndl
  Description: Checks if the mutex handle refers to a valid mutex
  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_mutex_hndl(mrapi_mutex_hndl_t mutex,
                                              mrapi_status_t* status) 
  {
    *status = MRAPI_ERR_MUTEX_INVALID;
    if (mrapi_impl_valid_lock_hndl(mutex,status)) {
      *status = MRAPI_SUCCESS;
      return MRAPI_TRUE;
    }
    return MRAPI_FALSE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_rwl_hndl
  Description: Checks if the rwl handle refers to a valid rwl
  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_rwl_hndl(mrapi_rwl_hndl_t rwl,
                                            mrapi_status_t* status) 
  {
    *status = MRAPI_ERR_RWL_INVALID;
    if (mrapi_impl_valid_lock_hndl(rwl,status)) {
      *status = MRAPI_SUCCESS;
      return MRAPI_TRUE;
    }
    return MRAPI_FALSE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_sem_hndl
  Description: Checks if the sem handle refers to a valid semaphore
  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/ 
  mrapi_boolean_t mrapi_impl_valid_sem_hndl (mrapi_sem_hndl_t sem, 
                                             mrapi_status_t* status) 
  {
    *status = MRAPI_ERR_SEM_INVALID;
    if (mrapi_impl_valid_lock_hndl(sem,status)) {
      *status = MRAPI_SUCCESS;
      return MRAPI_TRUE;
    }
    return MRAPI_FALSE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_lock_hndl
  Description: Checks if the sem handle refers to a valid semaphore
  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_lock_hndl (mrapi_sem_hndl_t sem,
                                              mrapi_status_t* status) {
    uint16_t s,d;
    mrapi_boolean_t rc = MRAPI_FALSE;
    

    if (!mrapi_impl_decode_hndl(sem,&s) || (s >= MRAPI_MAX_SEMS)) {
      return MRAPI_FALSE;
    }
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
    
    if (mrapi_db->sems[s].valid==MRAPI_TRUE)  {
      rc = MRAPI_TRUE;
    } else {
      /* check to see if it is a deleted sem that had extended error checking set */
      for (d = 0; d < MRAPI_MAX_SEMS; d++) {
        if ((mrapi_db->sems[d].deleted == MRAPI_TRUE) &&
            (mrapi_db->sems[d].handle == sem)) {
          switch (mrapi_db->sems[d].type) {
          case (MUTEX): *status = MRAPI_ERR_MUTEX_DELETED; break;
          case (SEM) :  *status = MRAPI_ERR_SEM_DELETED; break;
          case (RWL) :  *status = MRAPI_ERR_RWL_DELETED; break;
          };
          break; 
        }
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_shmem_hndl

  Description: Checks if the sem handle refers to a valid shmem segment

  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_shmem_hndl(mrapi_shmem_hndl_t shmem) 
  {
    uint16_t s;
    mrapi_boolean_t rc = MRAPI_TRUE;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    if (!mrapi_impl_decode_hndl(shmem,&s) || (s >= MRAPI_MAX_SHMEMS)) {
      rc = MRAPI_FALSE;
    }
    if ((rc) && (s < MRAPI_MAX_SHMEMS) && !mrapi_db->shmems[s].valid) {
      rc = MRAPI_FALSE;
    }
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function:mrapi_impl_valid_status_param
  Description: checks if the given status is a valid status parameter
  Parameters: status
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_status_param (const mrapi_status_t* mrapi_status)
  {
    return (mrapi_status != NULL);
  }
  
  /***************************************************************************
  Function:mrapi_impl_mutex_validID
  Description: checks if the given ID is valid
  Parameters: 
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_validID(mrapi_mutex_id_t mutex) { return MRAPI_TRUE;}
  
  /***************************************************************************
  Function:mrapi_impl_sem_validID
  Description: checks if the given ID is valid
  Parameters: 
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_validID(mrapi_sem_id_t sem) { return MRAPI_TRUE;}
  
  /***************************************************************************
  Function:mrapi_impl_rwl_validID
  Description: checks if the given ID is valid
  Parameters: 
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_validID(mrapi_rwl_id_t rwl) { return MRAPI_TRUE;}
  
  /***************************************************************************
  Function:mrapi_impl_shmem_validID
  Description: checks if the given ID is valid
  Parameters: 
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_validID(mrapi_shmem_id_t shmem) { return MRAPI_TRUE;}
  
  /***************************************************************************
  Function:mrapi_impl_rmem_validID
  Description: checks if the given ID is valid
  Parameters: 
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_validID(mrapi_rmem_id_t rmem) { return MRAPI_TRUE;}
  
  /***************************************************************************
  Function: mrapi_impl_initialized
  Description: checks if the given domain_id/node_id is already assoc w/ this pid/tid
  Parameters: node_id,domain_id
  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_initialized ()
  {
    mrapi_node_t node;
    mrapi_domain_t domain;
    uint32_t n,d;

    return mrapi_impl_whoami(&node,&n,&domain,&d);
  }  
  
  /***************************************************************************
  Function:mrapi_valid_node_num

  Description: checks if the given node_num is a valid node_num for this system

  Parameters: node_num: the node num to be checked

  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_node_num(mrapi_node_t node_num)
  {
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mcapi_impl_get_node_num

  Description: gets the node_num 

  Parameters: node_num: the pointer to be filled in

  Returns: boolean indicating success (the node num was found) or failure
   (couldn't find the node num).
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_get_node_num(mrapi_node_t* node)
  {
    uint32_t n,d;
    mrapi_domain_t domain;
    
    return mrapi_impl_whoami(node,&n,&domain,&d);
  }  
  
  /***************************************************************************
  Function: mcapi_impl_get_domain_num

  Description: gets the domain_num 

  Parameters: domain_num: the pointer to be filled in

  Returns: boolean indicating success (the node num was found) or failure
   (couldn't find the node num).
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_get_domain_num(mrapi_domain_t* domain)
  {
    uint32_t n,d;
    mrapi_node_t node;
    
    return mrapi_impl_whoami(&node,&n,domain,&d);
  }    
  
  /***************************************************************************
  Function:mrapi_valid_domain_num

  Description: 

  Parameters:

  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_domain_num(mrapi_domain_t domain_num)
  {
    return MRAPI_TRUE;
  }  
  
  /***************************************************************************
  Function:mrapi_impl_finalize node

  Description: 

  Parameters:

  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_finalize_node_locked (uint32_t d, uint32_t n) 
  {
    
    mrapi_boolean_t rc = MRAPI_TRUE;
    int shmem,sem;
    
    mrapi_dprintf(2,"mrapi_impl_finalize_node_locked dindex=%d nindex=%d domain=%d node=%d",
                  d,
                  n,
                  mrapi_db->domains[d].domain_id,
                  mrapi_db->domains[d].nodes[n].node_num);
    /* mark the node as finalized */
    mrapi_db->domains[d].nodes[n].valid = MRAPI_FALSE;
    
    /* decrement the shmem reference count if necessary */
    for (shmem = 0; shmem < MRAPI_MAX_SHMEMS; shmem++) {
      if (mrapi_db->shmems[shmem].valid == MRAPI_TRUE) { 
        if (mrapi_db->domains[d].nodes[n].shmems[shmem] == 1) {
          /* if this node was a user of this shm, decrement the ref count */
          mrapi_db->shmems[shmem].refs--;
        } 
        /* if the reference count is 0, free the shared memory resource */
        if (mrapi_db->shmems[shmem].refs == 0) {
          rc |= sys_shmem_detach(mrapi_db->shmems[shmem].addr);
          /* note: this will only work if no-one is still attached to it */
          rc |= sys_shmem_delete(mrapi_db->shmems[shmem].id);
          if (!rc) {
            fprintf(stderr,"mrapi_impl_finalize_node_locked: ERROR: sys_shmem_detach/delete failed\n");
          }
        }
      }
    }
    
    /* decrement the sem reference count if necessary */
    for (sem = 0; sem < MRAPI_MAX_SEMS; sem++) {
      if (mrapi_db->sems[sem].valid == MRAPI_TRUE) { 
        if (mrapi_db->domains[d].nodes[n].sems[sem] == 1) {
          /* if this node was a user of this sem, decrement the ref count */
          mrapi_db->sems[sem].refs--;
        } 
        /* if the reference count is 0 free the resource */
        if (mrapi_db->sems[sem].refs == 0) {
          mrapi_db->sems[sem].valid = MRAPI_FALSE;
        }
      }
    }
    
    return rc;
  }
  
  /***************************************************************************
  Function:mrapi_impl_free_resources

  Description: 

  Parameters:

  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_free_resources(mrapi_boolean_t panic) 
  {
    mrapi_boolean_t rc = MRAPI_TRUE;
    uint32_t d,n;
    mrapi_domain_t domain_num;
    mrapi_node_t node;
    pid_t pid = getpid();
    mrapi_boolean_t last_man_standing = MRAPI_TRUE;
    mrapi_boolean_t last_man_standing_for_this_process = MRAPI_TRUE;
    mrapi_boolean_t locked;
            
    /* try to lock the database */
    locked = mrapi_impl_access_database_pre(semid,0,MRAPI_FALSE);
    mrapi_dprintf(1,"mrapi_impl_free_resources (panic=%d): freeing any existing resources in the database mrapi_db=%p semid=%x shmemid=%x\n",
                  panic,mrapi_db,semid,shmemid);
    
    if (mrapi_db) {
      
      /* finalize this node */
      if (mrapi_impl_whoami(&node,&n,&domain_num,&d)) {
        mrapi_impl_finalize_node_locked(d,n);
      }    
      
      /* if we are in panic mode, then forcefully finalize all other nodes that belong to this process */
      if (panic) { 
        for (d = 0; d < MRAPI_MAX_DOMAINS; d++) {
          for (n = 0; n < MRAPI_MAX_NODES; n++) {
            if ((mrapi_db->domains[d].nodes[n].valid == MRAPI_TRUE) &&
                (mrapi_db->domains[d].nodes[n].pid == pid)) {
              mrapi_impl_finalize_node_locked(d,n);
            }
          }
        }
      }
      
      /* see if there are any valid nodes left in the system and for this process */
      for (d = 0; d < MRAPI_MAX_DOMAINS; d++) {
        for (n = 0; n < MRAPI_MAX_NODES; n++) {
          if (mrapi_db->domains[d].nodes[n].valid == MRAPI_TRUE) {
            last_man_standing = MRAPI_FALSE;
            if (mrapi_db->domains[d].nodes[n].pid == pid) {
              last_man_standing_for_this_process = MRAPI_FALSE;
            }
          }
        }
      }
      
      if (panic) {
        mrapi_assert(last_man_standing_for_this_process);
      }
      
       /* if there are no other valid nodes in the whole system, then free the sems */
      if (last_man_standing) {
        mrapi_dprintf(1,"mrapi_impl_free_resources: freeing mrapi internal semaphore and shared memory\n");
        
        /* free the mrapi internal semaphores */
        if (sems_semid != semid) {
          rc = sys_sem_delete(sems_semid);
          sems_semid = -1;
          if (!rc) {
            fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_sem_delete (mrapi_db) failed\n");
          }
        }

        if (shmems_semid != semid) {
          rc = sys_sem_delete(shmems_semid);
          shmems_semid = -1;
          if (!rc) {
            fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_sem_delete (mrapi_db) failed\n");
          }
        }
        if (rmems_semid != semid) {
          rc = sys_sem_delete(rmems_semid);
          rmems_semid = -1;
          if (!rc) {
            fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_sem_delete (mrapi_db) failed\n");
          }
        }
        if (requests_semid != semid) {
          rc = sys_sem_delete(requests_semid);
          requests_semid = -1;
          if (!rc) {
            fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_sem_delete (mrapi_db) failed\n");
          }
        }
        /* free the global semaphore last */
        rc = sys_sem_delete(semid);
        semid = -1;
        if (!rc) {
          fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_sem_delete (mrapi_db) failed\n");
        }
      }
      
      
      /* if there are no other valid nodes for this process, then detach from shared memory */
      if (last_man_standing_for_this_process) {
        
        /* detach from the mrapi internal shared memory */
        mrapi_dprintf(1,"mrapi_impl_free_resources: detaching from mrapi internal shared memory\n");
        
        rc = sys_shmem_detach (mrapi_db);
        if (!rc) {
          fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_shmem detach (mrapi_db) failed\n");
        }  
      }
      
      /* if there are no other valid nodes in the whole system, then free the shared memory */
      if (last_man_standing) {
        /* free the mrapi internal shared memory */
        rc = sys_shmem_delete(shmemid);
        if (!rc) {
          fprintf(stderr,"mrapi_impl_free_resources: ERROR: sys_shmem_delete (mrapi_db) failed\n");
        }
        //mrapi_db = NULL;
        shmemid = -1;
      }
      
      /* if we locked the database and didn't delete it, then we need to unlock it */
      if (locked && !last_man_standing) {
        /* unlock the database */
        mrapi_assert(mrapi_impl_access_database_post(semid,0));
      }
    }
 
    return last_man_standing;
  }
  
  /***************************************************************************
  Function:mrapi_impl_finalize

  Description: 

  Parameters:

  Returns:MRAPI_TRUE/MRAPI_FALSE
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_finalize()
  {
    mrapi_impl_free_resources(MRAPI_FALSE);
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function:mrapi_impl_valid_parameters_param

  Description: checks that the parameter is valid
  
  Parameters: 

  Returns:  boolean indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_parameters_param (mrapi_parameters_t mrapi_parameters)
  {
    return MRAPI_TRUE;
  }

  /***************************************************************************
  Function:mrapi_impl_valid_info_param

  Description: checks that the parameter is valid

  Parameters:

  Returns:  boolean indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_info_param (const mrapi_info_t* mrapi_info)
  {
    return (mrapi_info != NULL);
  }
  
  /***************************************************************************
  NAME:mrapi_impl_signal_hndlr 
  DESCRIPTION: The purpose of this function is to catch signals so that we
   can clean up our shared memory and sempaphore resources cleanly.
  PARAMETERS: the signal
  RETURN VALUE: none
  ***************************************************************************/
  void mrapi_impl_signal_hndlr ( int sig ) 
  {
    struct sigaction old_action,new_action;
    
    mca_block_signals();
    
    /* print info on which signal was caught */
#ifdef __linux
    char sigbuff[128];
    sprintf(sigbuff,"SIGNAL: mrapi received signal[%d] pid=%d tid=%s dindex=%d nindex=%d mrapi_db=%p",
            sig,mrapi_pid,mca_print_tid(mrapi_tid),mrapi_dindex, mrapi_nindex,mrapi_db);
    psignal(sig,sigbuff); 
    
#else
    printf("mrapi received signal: %s\n",strsignal(sig));
#endif 
    /* restore the old action */
    if (mrapi_db != NULL) {
      
      mrapi_impl_access_database_pre(semid,0,MRAPI_FALSE);
      old_action = mrapi_db->domains[mrapi_dindex].nodes[mrapi_nindex].signals[sig];
      mrapi_impl_access_database_post(semid,0);
      sigaction (sig, &old_action, NULL);
    } else {
      mrapi_dprintf(1, "MRAPI: Unable to look up node/domain info for this process, thus unable to unwind the signal handlers any further. Restoring the default handler\n");
      new_action.sa_handler = SIG_DFL;
      sigemptyset (&new_action.sa_mask);
      new_action.sa_flags = 0;
      sigaction (sig, &new_action, NULL);
    }
    
    /* clean up ipc resources */
    mrapi_impl_free_resources(MRAPI_TRUE);
    
    mca_unblock_signals();
    
    /* Now reraise the signal. */ 
    raise (sig);
  }
  
  /***************************************************************************
   Function: mrapi_impl_create_sys_semaphore
   
   Description: Create or get the semaphore corresponding to the key
   
   Parameters: 
   
   Returns: boolean indicating success or failure
   
***************************************************************************/
  mrapi_boolean_t mrapi_impl_create_sys_semaphore (int* id, 
                                                   int num_locks, 
                                                   int key, 
                                                   mrapi_boolean_t lock) 
  {
    unsigned max_tries = 0xffffffff;
    unsigned trycount = 0;
   
    while (trycount < max_tries) {
      trycount++;
      if ((sys_sem_create(key,num_locks,id) || sys_sem_get(key,num_locks,id))) {  
        if (!lock) { 
          return MRAPI_TRUE;
        } else if (sys_sem_trylock(*id,0)) {
          return MRAPI_TRUE;
        }
      }
    }
    return MRAPI_FALSE;
  }

  /***************************************************************************
   Function: mrapi_impl_initialize
   
   Description:initializeZs the mrapi_impl layer (sets up the database and semaphore)
   
   Parameters: 
   
   Returns: boolean indicating success or failure
   
***************************************************************************/
  mrapi_boolean_t mrapi_impl_initialize (mrapi_domain_t domain_id, 
                                         mrapi_node_t node_id, 
                                         mrapi_status_t* status) 
  {
    /* associate this node w/ a pid,tid pair so that we can recognize the
       caller on later calls */
    
    int d = 0;
    int n = 0;
    mrapi_boolean_t rc = MRAPI_TRUE;
    int semid_local;
    int shmemid_local;
    int key;
    int sems_key;
    int shmems_key;
    int rmems_key;
    int requests_key;
    mrapi_boolean_t created_shmem = MRAPI_TRUE;
    register struct passwd *pw;
    register uid_t uid;
    char buff[128];
    mrapi_boolean_t use_uid = MRAPI_TRUE;
    
    mrapi_dprintf(1,"mrapi_impl_initialize (%d,%d);",domain_id,node_id);
       
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
      strcat(buff,"_mrapi");
      /* global key */
      key = 0;
      key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
      /* sems key */
      strcat(buff,"_sems");
      sems_key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
      /* shmems key */
      strcat(buff,"_shmems");
      shmems_key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
      /* rmems key */
      strcat(buff,"_rmems");
      rmems_key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
      /* requests key */
      strcat(buff,"_requests");
      requests_key =  mca_Crc32_ComputeBuf( key,buff,sizeof(buff));
    } else {
      key = ftok("/dev/null",'c');
      sems_key = key + 10;
      shmems_key = key + 20;
      rmems_key = key + 30;
      requests_key = key + 40;
    }
    
    /* 1) setup the global database */  
    /* create/get the shared memory database */
    shmemid_local = sys_shmem_create(key,sizeof(mrapi_database));
    if (shmemid_local == -1) {
      created_shmem = MRAPI_FALSE;
      shmemid_local = sys_shmem_get(key,sizeof(mrapi_database));
    }
    /* attach to the shared memory */
    if (shmemid_local != -1) {
      /* setup the global mrapi_db pointer and global shmemid */
      /* FIXME: IS IT SAFE TO WRITE THIS GLOBAL W/O A LOCK ??? */
      mrapi_db = (mrapi_database*)sys_shmem_attach(shmemid_local);
    } 
    if (mrapi_db == NULL) {
      fprintf(stderr,"MRAPI_ERROR: Unable to attached to shared memory key= %x, errno=%s",
              key,strerror(errno));
      rc = MRAPI_FALSE;
    }
    
    /* 2) create or get the semaphore and lock it */
    /* we loop here because of the following race condition:
       initialize                  finalize
       1) create/get sem           1) lock sem
       2) lock sem                 2) check db: any valid nodes?
       3) setup db & add self      3a)  no -> delete db & delete sem
       4) unlock sem               3b)  yes-> unlock sem
       
       finalize-1 can occur between initialize-1 and initialize-2 which will cause initilize-2
       to fail because the semaphore no longer exists.
    */
    if (!mrapi_impl_create_sys_semaphore(&semid_local,1/*num_locks*/,key,MRAPI_TRUE)) {
      fprintf(stderr,"MRAPI ERROR: Unable to get the semaphore key= %x, errno=%s\n",
              key,strerror(errno));
      rc = MRAPI_FALSE;
    }
       
    if (rc) {
      mrapi_dprintf(1,"mrapi_impl_initialize lock acquired, now attaching to shared memory and adding node to database");
      shmemid = shmemid_local;
      /* At this point we've managed to acquire and lock the semaphore ...*/
      /* NOTE: it's important to write to the globals only while
         we have the semaphore otherwise we introduce race conditions.  This
         is why we are using the local variable id until everything is set up.*/
      
      /* set the global semid */
      semid = semid_local;
      
      /* get or create our finer grained locks */
      /* in addition to a lock on the sems array, every lock (rwl,sem,mutex) has it's own 
         database semaphore, this allows us to access different locks in parallel */
      if (use_global_only || 
          !mrapi_impl_create_sys_semaphore(&sems_semid,MRAPI_MAX_SEMS+1/*num_locks*/,sems_key,MRAPI_FALSE)) {
        sems_semid = semid;
      }
      if (use_global_only || 
          !mrapi_impl_create_sys_semaphore(&shmems_semid,1/*num_locks*/,shmems_key,MRAPI_FALSE)) {
        shmems_semid = semid;
      }
      if (use_global_only || 
          !mrapi_impl_create_sys_semaphore(&rmems_semid,1/*num_locks*/,rmems_key,MRAPI_FALSE)) {
        rmems_semid = semid;
      }
      if (use_global_only || 
          !mrapi_impl_create_sys_semaphore(&requests_semid,1/*num_locks*/,requests_key,MRAPI_FALSE)) {
        requests_semid = semid;
      }
      
      
      //printf("**** semid:%x sems:%x shmems:%x rmems:%x requests:%x\n",
             //key,sems_key,shmems_key,rmems_key,requests_key);
      
      /* 3) add the node/domain to the database */
      if (rc) {
        /* first see if this domain already exists */
        for (d = 0; d < MRAPI_MAX_DOMAINS; d++) {
          if (mrapi_db->domains[d].domain_id == domain_id) {
            break;
          }
        }
        if (d == MRAPI_MAX_DOMAINS) {
          /* it didn't exist so find the first available entry */
          for (d = 0; d < MRAPI_MAX_DOMAINS; d++) {
            if (mrapi_db->domains[d].valid == MRAPI_FALSE) {
              break;
            }
          }
        }
        if (d != MRAPI_MAX_DOMAINS) {
          /* now find an available node index...*/
          for (n = 0; n < MRAPI_MAX_NODES; n++) {
            /* Even though initialized() is checked by mrapi, we have to check again here because 
               initialized() and initalize() are  not atomic at the top layer */
            if ((mrapi_db->domains[d].nodes[n].valid )&& 
                (mrapi_db->domains[d].nodes[n].node_num == node_id)) {
              /* this node already exists for this domain */
              rc = MRAPI_FALSE;
              *status = MRAPI_ERR_NODE_INITFAILED;
              mrapi_dprintf(1,"This node (%d) already exists for this domain(%d)",node_id,domain_id);
              break; 
            }
          }
          if (n == MRAPI_MAX_NODES) {
            /* it didn't exist so find the first available entry */
            for (n = 0; n < MRAPI_MAX_NODES; n++) {
            if (mrapi_db->domains[d].nodes[n].valid == MRAPI_FALSE)
              break;
            }
          }
        } else {
          /* we didn't find an available domain index */
          mrapi_dprintf(1,"You have hit MRAPI_MAX_DOMAINS, either use less domains or reconfigure with more domains");
          rc = MRAPI_FALSE;
        }
        if (n == MRAPI_MAX_NODES) {
          /* we didn't find an available node index */
          mrapi_dprintf(1,"You have hit MRAPI_MAX_NODES, either use less nodes or reconfigure with more nodes.");
          rc = MRAPI_FALSE;
        }
      }
      
      if (rc) {
        mrapi_dprintf(1,"adding domain_id:%x node_id:%x to d:%d n%d",
                      domain_id,node_id,d,n);
        mrapi_assert (mrapi_db->domains[d].nodes[n].valid == MRAPI_FALSE); 
        mrapi_db->domains[d].domain_id = domain_id;
        mrapi_db->domains[d].valid = MRAPI_TRUE;
        mrapi_db->domains[d].num_nodes++;
        mrapi_db->domains[d].nodes[n].valid = MRAPI_TRUE;
        mrapi_db->domains[d].nodes[n].node_num = node_id;
        /* set our cached (thread-local-storage) identity */
        mrapi_pid = getpid();
        mrapi_tid = pthread_self(); 
        mrapi_nindex = n;
        mrapi_dindex = d;
        mrapi_domain_id = domain_id;
        mrapi_node_id = node_id;
        mrapi_db->domains[d].nodes[n].pid = mrapi_pid;
        mrapi_db->domains[d].nodes[n].tid = mrapi_tid;
        
        /* Set the resource tree pointer */
        resource_root = &chip;
        
        mrapi_dprintf(1,"registering signal handlers"); 
        /* register signal handlers so that we can still clean up resources 
           if an interrupt occurs 
           http://www.gnu.org/software/libtool/manual/libc/Sigaction-Function-Example.html
        */
        struct sigaction new_action, old_action;
        
        /* Set up the structure to specify the new action. */
        new_action.sa_handler = mrapi_impl_signal_hndlr;
        sigemptyset (&new_action.sa_mask);
        new_action.sa_flags = 0;
        
        sigaction (SIGINT, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGINT] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGINT, &new_action, NULL);
        
        sigaction (SIGHUP, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGHUP] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGHUP, &new_action, NULL);
        
        sigaction (SIGILL, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGILL] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGILL, &new_action, NULL);  
        
        sigaction (SIGSEGV, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGSEGV] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGSEGV, &new_action, NULL);
        
        sigaction (SIGTERM, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGTERM] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGTERM, &new_action, NULL);
        
        sigaction (SIGFPE, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGFPE] = old_action;
        if (old_action.sa_handler != SIG_IGN)
          sigaction (SIGFPE, &new_action, NULL);  
        
        sigaction (SIGABRT, NULL, &old_action);
        mrapi_db->domains[d].nodes[n].signals[SIGABRT] = old_action;
        if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGABRT, &new_action, NULL);
        
      }
      
      /* release the lock */
      mrapi_impl_access_database_post(semid_local,0);
    }
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_create
  
  Description:
  
  Parameters:
  
  Returns:  boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_create(mrapi_mutex_hndl_t* mutex,  
                                          mrapi_mutex_id_t mutex_id,
                                          const mrapi_mutex_attributes_t* attributes, 
                                          mrapi_status_t* mrapi_status)
  {
    uint16_t m;
    *mrapi_status = MRAPI_ERR_MUTEX_LIMIT; 
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    /* lock the database (use global lock for get/create sem|rwl|mutex) */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));
    
    if (mrapi_impl_create_lock_locked(mutex,mutex_id,1,MUTEX,mrapi_status)) {
      mrapi_assert (mrapi_impl_decode_hndl(*mutex,&m));
      mrapi_db->sems[m].type = MUTEX;
      if (attributes != NULL) {
        /* set the user-specified attributes */
        mrapi_db->sems[m].attributes.recursive = attributes->recursive;
        mrapi_db->sems[m].attributes.ext_error_checking = attributes->ext_error_checking;
        mrapi_db->sems[m].attributes.shared_across_domains = attributes->shared_across_domains;
      } else {
        /* set the default attributes */
        mrapi_db->sems[m].attributes.recursive = MRAPI_FALSE;
        mrapi_db->sems[m].attributes.ext_error_checking = MRAPI_FALSE;
        mrapi_db->sems[m].attributes.shared_across_domains = MRAPI_TRUE;
      }
      rc = MRAPI_TRUE;
    }
    /* unlock the database (use global lock for get/create sem|rwl|mutex) */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_init_attributes

  Description:

  Parameters:

  Returns:  boolean

  ***************************************************************************/
  void mrapi_impl_mutex_init_attributes(mrapi_mutex_attributes_t* attributes) 
  {
    attributes->recursive = MRAPI_FALSE;
    attributes->ext_error_checking = MRAPI_FALSE;
    attributes->shared_across_domains = MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_set_attribute

  Description:

  Parameters:

  Returns:  boolean
  ***********************************************************************/
  void mrapi_impl_mutex_set_attribute (mrapi_mutex_attributes_t* attributes,
                                       mrapi_uint_t attribute_num, 
                                       const void* attribute,
                                       size_t attr_size, 
                                       mrapi_status_t* status)
  {
    
    switch(attribute_num) {
    case (MRAPI_MUTEX_RECURSIVE):
      if (attr_size != sizeof(attributes->recursive)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->recursive,(mrapi_boolean_t*)attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_ERROR_EXT):
      if (attr_size != sizeof(attributes->ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->ext_error_checking,(mrapi_boolean_t*)attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED):
      if (attr_size != sizeof(attributes->shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->shared_across_domains,(mrapi_boolean_t*)attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
  }

  /***************************************************************************
  Function: mrapi_impl_mutex_get_attribute

  Description:

  Parameters:

  Returns:  boolean
  ***********************************************************************/
  void mrapi_impl_mutex_get_attribute (mrapi_mutex_hndl_t mutex,
                                       mrapi_uint_t attribute_num, 
                                       void* attribute,
                                       size_t attr_size, 
                                       mrapi_status_t* status)
  {
    uint16_t m;
    mrapi_assert (mrapi_impl_decode_hndl(mutex,&m));
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,m,MRAPI_TRUE));
    
    switch(attribute_num) {
    case (MRAPI_MUTEX_RECURSIVE):
      if (attr_size != sizeof(mrapi_db->sems[m].attributes.recursive)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[m].attributes.recursive,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_ERROR_EXT):
      if (attr_size != sizeof(mrapi_db->sems[m].attributes.ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[m].attributes.ext_error_checking,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED):
      if (attr_size != sizeof(mrapi_db->sems[m].attributes.shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[m].attributes.shared_across_domains,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,m));
  }
  
  /***************************************************************************
  Function:  mrapi_impl_mutex_get
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_get (mrapi_mutex_hndl_t* mutex, 
                                        mrapi_sem_id_t key) 
  {
    return mrapi_impl_sem_get(mutex,key);
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_delete
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_delete (mrapi_mutex_hndl_t mutex) 
  { 
    return mrapi_impl_sem_delete(mutex);
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_lock
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_lock(mrapi_mutex_hndl_t mutex,
                                        mrapi_key_t* lock_key, 
                                        mrapi_timeout_t timeout, 
                                        mrapi_status_t* mrapi_status)
  {
    uint32_t n = 0;
    uint32_t d = 0;
    uint16_t m = 0;
    mrapi_boolean_t rc = MRAPI_FALSE;
    mca_node_t node_id;
    mca_domain_t domain_id;
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    mrapi_assert (mrapi_impl_decode_hndl(mutex,&m));
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,m,MRAPI_TRUE));
        
    if ( (mrapi_db->sems[m].locks[0].locked == MRAPI_TRUE) &&
         (mrapi_db->sems[m].locks[0].lock_holder_nindex == n) &&
         (mrapi_db->sems[m].locks[0].lock_holder_dindex == d)) {
      
      if (mrapi_db->sems[m].attributes.recursive == MRAPI_TRUE) {
        mrapi_db->sems[m].locks[0].lock_key++;
        *lock_key = mrapi_db->sems[m].locks[0].lock_key;
        rc=MRAPI_TRUE;
        mrapi_dprintf(1,"mrapi_impl_mutex_lock rc=TRUE (recursive) (0x%x,%d /*lock_key*/,%d /*timeout*/,&status);",
                      mutex,*lock_key,timeout);
      } else {
        mrapi_dprintf(1,"WARNING: attempting to lock a non-recursive mutex when you already have the lock!");
        /* unlock the database */
        mrapi_assert(mrapi_impl_access_database_post(sems_semid,m));
        return MRAPI_FALSE;
      }
    }
    
    /* unlock the database */
   mrapi_assert(mrapi_impl_access_database_post(sems_semid,m));
    
   if (!rc) {
     rc = mrapi_impl_sem_lock(mutex,1,timeout,mrapi_status);
     if (rc) {
       *lock_key = 0;
       mrapi_dprintf(1,"mrapi_impl_mutex_lock rc=TRUE (not recursive/first lock) (0x%x,%d /*lock_key*/,%d /*timeout*/,&status);",mutex,*lock_key,timeout);
     } else {
       mrapi_dprintf(1,"mrapi_impl_mutex_lock rc=FALSE (0x%x,%d /*lock_key*/,%d /*timeout*/,&status);",
                     mutex,*lock_key,timeout); 
     }
   }
    
   return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_mutex_unlock
                             
  Description:
              
  Parameters:
             
  Returns:  boolean indicating success or failure
             
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_mutex_unlock(mrapi_mutex_hndl_t mutex,
                                          const mrapi_key_t* lock_key,
                                          mrapi_status_t* mrapi_status) 
  { 
    uint32_t n = 0;
    uint32_t d = 0;
    uint16_t m;
    mrapi_boolean_t rc = MRAPI_FALSE;
    mca_node_t node_id;
    mca_domain_t domain_id;
    mrapi_boolean_t recursively_locked;
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    mrapi_assert (mrapi_impl_decode_hndl(mutex,&m));

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,m,MRAPI_TRUE));
        
    mrapi_dprintf(1,"mrapi_impl_mutex_unlock (0x%x,%d /*lock_key*/,&status);",mutex,*lock_key);
    
    if ((mrapi_db->sems[m].locks[0].lock_holder_nindex == n) &&
        (mrapi_db->sems[m].locks[0].lock_holder_dindex == d) && 
        (*lock_key == mrapi_db->sems[m].locks[0].lock_key) &&
        (*lock_key != 0) ) {
      
      mrapi_db->sems[m].locks[0].lock_key--;
      rc=MRAPI_TRUE;
    } 
    
    recursively_locked = mrapi_db->sems[m].locks[0].lock_key;
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,m));
    
    if ((!rc) && (!recursively_locked)) {
      rc =  mrapi_impl_release_lock(mutex,1,mrapi_status);
    }
    
    if ((!rc) && (recursively_locked)) { 
      *mrapi_status = MRAPI_ERR_MUTEX_LOCKORDER;
    }
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_create
  
  Description:
  
  Parameters:
  
  Returns:  boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_create(mrapi_rwl_hndl_t* rwl,  
                                        mrapi_rwl_id_t rwl_id,
                                        const mrapi_rwl_attributes_t* attributes, 
                                        mrapi_uint32_t reader_lock_limit,
                                        mrapi_status_t* mrapi_status)
  {
    *mrapi_status = MRAPI_ERR_RWL_LIMIT;
    uint16_t r;
    mrapi_boolean_t rc = MRAPI_FALSE;

    /* lock the database (use global lock for get/create sem|rwl|mutex) */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    if ( mrapi_impl_create_lock_locked(rwl,rwl_id,reader_lock_limit,RWL,mrapi_status)) {
      mrapi_assert (mrapi_impl_decode_hndl(*rwl,&r));
      mrapi_db->sems[r].type = RWL;
      if (attributes != NULL) {
        /* set the user-specified attributes */
        mrapi_db->sems[r].attributes.ext_error_checking = attributes->ext_error_checking;
        mrapi_db->sems[r].attributes.shared_across_domains = attributes->shared_across_domains;
      } else {
        /* set the default attributes */
        mrapi_db->sems[r].attributes.ext_error_checking = MRAPI_FALSE;
        mrapi_db->sems[r].attributes.shared_across_domains = MRAPI_TRUE;
      }
      rc = MRAPI_TRUE;
    }
    /* unlock the database (use global lock for get/create sem|rwl|mutex)*/
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_init_attributes
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
    
  ***************************************************************************/
  void mrapi_impl_rwl_init_attributes(mrapi_rwl_attributes_t* attributes) 
  {
    attributes->ext_error_checking = MRAPI_FALSE;
    attributes->shared_across_domains = MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_set_attribute
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_rwl_set_attribute (mrapi_rwl_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, 
                                     const void* attribute,
                                     size_t attr_size, 
                                     mrapi_status_t* status)
  {  
    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(attributes->ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->ext_error_checking,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(attributes->shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->shared_across_domains,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_get_attribute
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_rwl_get_attribute (mrapi_rwl_hndl_t rwl, 
                                     mrapi_uint_t attribute_num, 
                                     void* attribute,
                                     size_t attr_size, 
                                     mrapi_status_t* status)
  {
    uint16_t r;
    
    mrapi_assert (mrapi_impl_decode_hndl(rwl,&r));
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,r,MRAPI_TRUE));

    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(mrapi_db->sems[r].attributes.ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[r].attributes.ext_error_checking,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(mrapi_db->sems[r].attributes.shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[r].attributes.shared_across_domains,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,r));
  }
  
  /***************************************************************************
  Function:  mrapi_impl_rwl_get
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_get (mrapi_rwl_hndl_t* rwl, mrapi_sem_id_t key) 
  {
    return mrapi_impl_sem_get(rwl,key);
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_delete
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_delete (mrapi_rwl_hndl_t rwl) 
  { 
    return mrapi_impl_sem_delete(rwl);
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_lock
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_lock(mrapi_rwl_hndl_t rwl,
                                      mrapi_rwl_mode_t mode, 
                                      mrapi_timeout_t timeout, 
                                      mrapi_status_t* mrapi_status)
  {
    uint16_t s;
    int n;
    mrapi_boolean_t rc = MRAPI_FALSE;    
    mrapi_dprintf(1,"mrapi_impl_rwl_lock (0x%x,%d /*mode*/,%d /*timeout*/,&status);",
                  rwl,mode,timeout);
    
    if (!mrapi_impl_decode_hndl(rwl,&s)) {
      return MRAPI_FALSE;
    }
    
    if (mode == MRAPI_RWL_READER) {
      /* a reader (shared) lock is being requested... */
      rc= mrapi_impl_sem_lock(rwl,1,timeout,mrapi_status);
    } else {
      /* a writer (exclusive) lock is being requested...*/
      
      /* lock the database */
      mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
      
      n = mrapi_db->sems[s].shared_lock_limit;
      
      /* unlock the database */
      mrapi_assert(mrapi_impl_access_database_post(sems_semid,s));
      /* request all the locks */
      rc = mrapi_impl_sem_lock(rwl,n,timeout,mrapi_status);
    }
  return rc; 
  }
  
  /***************************************************************************
  Function: mrapi_impl_rwl_unlock
    
  Description:
    
  Parameters:
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rwl_unlock(mrapi_rwl_hndl_t rwl,
                                        mrapi_rwl_mode_t mode,
                                        mrapi_status_t* mrapi_status) 
  { 
    uint16_t s;
    int n;
    
    mrapi_dprintf(1,"mrapi_impl_rwl_unlock (0x%x,%d /*mode*/,&status);",
                  rwl,mode);
    
    if (!mrapi_impl_decode_hndl(rwl,&s)) {
      return MRAPI_FALSE;
    }
    
    if (mode == MRAPI_RWL_READER) {
      /* a reader (shared) unlock is being requested...*/
      return mrapi_impl_sem_unlock(rwl,1,mrapi_status);
    } else {
      /* a writer (exclusive) unlock is being requested...*/
      /* lock the database */
      mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
      
      n = mrapi_db->sems[s].shared_lock_limit;
      
      /* unlock the database */
      mrapi_assert(mrapi_impl_access_database_post(sems_semid,s));
      
      /* release all the locks */
      return mrapi_impl_sem_unlock(rwl,n,mrapi_status);
    }
  }     
  
  /***************************************************************************
  Function: mrapi_impl_shmem_attached
    
  Description:  
    
  Parameters: 
    
  Returns: boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_attached (mrapi_shmem_hndl_t shmem) {
    uint16_t s;
    uint32_t d = 0;
    uint32_t n = 0;
    mrapi_boolean_t rc = MRAPI_FALSE;
    mrapi_node_t node_id;
    mrapi_domain_t domain_id;

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));

    /* make sure we recognize the caller */
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));

    if (mrapi_impl_decode_hndl(shmem,&s) && mrapi_db->shmems[s].valid && 
        ( mrapi_db->domains[d].nodes[n].shmems[s]==1) ) {
      rc = MRAPI_TRUE;
    }
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));

    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_shmem_exists
    
  Description:
    
  Parameters: shmemkey - the shared key for all users of this memory segment
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_exists(uint32_t shmemkey) 
  {
    
    mrapi_boolean_t rc = MRAPI_FALSE;
    unsigned s;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    
    /* look for a valid shmem w/ the same key */
    for (s = 0; s < MRAPI_MAX_SHMEMS; s++) {
      if ((mrapi_db->shmems[s].valid == MRAPI_TRUE) &&
          (mrapi_db->shmems[s].key == shmemkey)) {
        rc = MRAPI_TRUE;
        break;
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_get
    
  Description:
    
  Parameters: shmemkey - the shared key for all users of this memory segment
              size - the desired size (in bytes)
              
  Returns: the address of the shared memory segment
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_get(mrapi_shmem_hndl_t* shmem_hndl,uint32_t key) 
  {
    uint32_t s;
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_shmem_get (&shmem,0x%x /*shmem key*/);",key);
    
    /* now look for the shmemkey */
    for (s = 0; s < MRAPI_MAX_SHMEMS; s++) {
      if (mrapi_db->shmems[s].key == key) {  
        rc = MRAPI_TRUE;
        break;
      }
    }
    
    /* encode the handle */
    /* note: if we didn't find it, the handle will be invalid bc the indices will be max*/
    *shmem_hndl = mrapi_impl_encode_hndl(s);
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_shmem_create
    
  Description:
    
  Parameters: shmemkey - the shared key for all users of this memory segment
              size - the desired size (in bytes)
              
  Returns: the address of the shared memory segment
              
  ***************************************************************************/
  void mrapi_impl_shmem_create(mrapi_shmem_hndl_t* shmem,
                                          uint32_t key,
                                          uint32_t size,
                                          const mrapi_shmem_attributes_t* attributes,
                                          mrapi_status_t* mrapi_status) 
  {    
    /* create shared memory */
    uint32_t n = 0;
    uint32_t d = 0;
    uint32_t s = 0;
    int id;
    mca_node_t node_id;
    mca_domain_t domain_id;
    mrapi_boolean_t rc = MRAPI_TRUE;
     
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_shmem_create(&shmem,0x%x,%d,&attrs);",key,size);
   
    *mrapi_status = MRAPI_SUCCESS;
 
    /* make sure we recognize the caller */
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    /* make sure we have room in the database */
    if  (mrapi_db->num_shmems == MRAPI_MAX_SHMEMS){
      mrapi_dprintf(2,"mrapi_impl_shmem_create: error: num_shmems == MRAPI_MAX_SHMEMS");
      *mrapi_status = MRAPI_ERR_MEM_LIMIT; 
      rc = MRAPI_FALSE;
    } else {
      /* make sure the shared memory doesn't already exist */
      for (s=0; s < MRAPI_MAX_SHMEMS; s++) {
        if (mrapi_db->shmems[s].valid && mrapi_db->shmems[s].key == key) {
          mrapi_dprintf(2,"mrapi_impl_shmem_create: error: the shmem now exists");
          *mrapi_status = MRAPI_ERR_SHM_EXISTS; 
          rc = MRAPI_FALSE;
          break;          
        }
      }
    }
    
    if (rc) {
      /* sysvr4: try to create the shared memory */
      id = sys_shmem_create(key,size); 
      if (id == -1) {
        mrapi_dprintf(2,"mrapi_impl_shmem_create: error: OS failed to create shared memory");
        *mrapi_status = MRAPI_ERR_MEM_LIMIT;
        rc = MRAPI_FALSE;
      } 
    }
    
    if (rc) {
      /* update our database */
      for (s = 0; s < MRAPI_MAX_SHMEMS; s++) {
        if (mrapi_db->shmems[s].valid == MRAPI_FALSE) {
          memset(&mrapi_db->shmems[s],0,sizeof(mrapi_shmem_t));
          mrapi_dprintf(1,"adding shmem: id=%u to dindex=%u nindex=%u sindex=%u \n",id,d,n,s);
          mrapi_db->shmems[s].key = key;
          mrapi_db->shmems[s].id = id;
          mrapi_db->shmems[s].valid = MRAPI_TRUE;
          mrapi_db->shmems[s].addr = NULL; /* we'll fill this in when we attach */
          /* fill in the attributes */
          if (attributes != NULL) {
            /* set the user-specified attributes */
            mrapi_db->shmems[s].attributes.ext_error_checking = attributes->ext_error_checking;
            mrapi_db->shmems[s].attributes.shared_across_domains = attributes->shared_across_domains;
          } else {
            /* set the default attributes */
            mrapi_db->shmems[s].attributes.ext_error_checking = MRAPI_FALSE;
            mrapi_db->shmems[s].attributes.shared_across_domains = MRAPI_TRUE;
          }
          mrapi_db->num_shmems++;
          break;
        }
      }
    }
    
    if (!rc) {
      s = MRAPI_MAX_SHMEMS;
    }
    
    *shmem = mrapi_impl_encode_hndl(s);
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_init_attributes
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
    
  ***************************************************************************/
  void mrapi_impl_shmem_init_attributes(mrapi_shmem_attributes_t* attributes) 
  {
    attributes->ext_error_checking = MRAPI_FALSE;
    attributes->shared_across_domains = MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_set_attribute
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_shmem_set_attribute (mrapi_shmem_attributes_t* attributes, 
                                       mrapi_uint_t attribute_num, 
                                       const void* attribute,
                                       size_t attr_size, 
                                       mrapi_status_t* status)
  {  
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE))

    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(attributes->ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->ext_error_checking,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(attributes->shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->shared_across_domains,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;

    case(MRAPI_SHMEM_RESOURCE): 
      if (attr_size != sizeof(attributes->resource)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->resource,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case(MRAPI_SHMEM_ADDRESS): 
      if (attr_size != sizeof(attributes->mem_addr)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->mem_addr,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case(MRAPI_SHMEM_SIZE): 
      if (attr_size != sizeof(attributes->mem_size)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->mem_size,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;

    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0))
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_get_attribute
  
  Description:
  
  Parameters:
  
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_shmem_get_attribute (mrapi_shmem_hndl_t shmem, 
                                       mrapi_uint_t attribute_num, 
                                       void* attribute,
                                       size_t attr_size, 
                                       mrapi_status_t* status)
  {
    uint16_t s;
    
    mrapi_assert (mrapi_impl_decode_hndl(shmem,&s));
    
    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(mrapi_db->shmems[s].attributes.ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->shmems[s].attributes.ext_error_checking,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(mrapi_db->shmems[s].attributes.shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->shmems[s].attributes.shared_across_domains,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case(MRAPI_SHMEM_RESOURCE): 
      if (attr_size != sizeof(mrapi_db->shmems[s].attributes.resource)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->shmems[s].attributes.resource,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
  case(MRAPI_SHMEM_ADDRESS): 
      if (attr_size != sizeof(mrapi_db->shmems[s].attributes.mem_addr)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->shmems[s].attributes.mem_addr,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
  case(MRAPI_SHMEM_SIZE): 
      if (attr_size != sizeof(mrapi_db->shmems[s].attributes.mem_size)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->shmems[s].attributes.mem_size,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;

    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_attach
    
  Description: attach to the shared memory segment
    
  Parameters: 
    
  Returns: The address of the shared memory segment or NULL if it fails.
    
  ***************************************************************************/
  void* mrapi_impl_shmem_attach (mrapi_shmem_hndl_t shmem) 
  {
    uint16_t s;
    uint32_t n = 0;
    uint32_t d = 0;
    mca_node_t node_id;
    mca_domain_t d_id;
    
    /* lock the database */
   mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&d_id,&d)); 
    
    mrapi_dprintf(1,"mrapi_impl_shmem_attach(0x%x);",shmem);
    mrapi_assert (mrapi_impl_decode_hndl(shmem,&s));
    /* fill in the addr */
    mrapi_db->shmems[s].addr = 
      sys_shmem_attach(mrapi_db->shmems[s].id);

    if (mrapi_db->domains[d].nodes[n].shmems[s] == 0) {   
      mrapi_db->domains[d].nodes[n].shmems[s] = 1; /* log this node as a user */
      mrapi_db->shmems[s].refs++; /* bump the reference count */
    }
    
   /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    
    return mrapi_db->shmems[s].addr;
  }
  
  /***************************************************************************
  Function: mrapi_delete_sharedMem
    
  Description: 
    
  Parameters: shmem_address - the address of the shared memory segment
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_delete(mrapi_shmem_hndl_t shmem) 
  {
    
    uint16_t s;
    mrapi_boolean_t rc = MRAPI_TRUE;
    
    /* lock the database */
   mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_shmem_delete(0x%x);",shmem);
    
    mrapi_assert (mrapi_impl_decode_hndl(shmem,&s));
   
    /* We do not check the reference count, it's up to the user to not delete
    shared mem that other nodes are using.  The reference count is only used
    by finalize to know if the last user of the shared mem has called finalize and
    if so, then it will delete it. */
 
    /* note: we can't actually free it unless no-one is attached to it */ 
    rc = sys_shmem_delete(mrapi_db->shmems[s].id);
    if (rc) {
      mrapi_db->shmems[s].valid = MRAPI_FALSE;
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_shmem_detach
   
  Description: 
   
  Parameters: shmem_address - the address of the shared memory segment
   
  Returns:  boolean indicating success or failure
   
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_shmem_detach(mrapi_shmem_hndl_t shmem) 
  { 
    uint16_t s;
    uint32_t d = 0;
    uint32_t n = 0;
    mrapi_domain_t domain_id;
    mrapi_node_t node_id;
    
    mrapi_boolean_t rc = MRAPI_FALSE;
    /* lock the database */
   mrapi_assert(mrapi_impl_access_database_pre(shmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    mrapi_dprintf(1,"mrapi_impl_shmem_detach(handle=0x%x);",shmem);
    
    mrapi_assert (mrapi_impl_decode_hndl(shmem,&s));
    
    /* look up the id that corresponds to this address */
    rc = sys_shmem_detach(mrapi_db->shmems[s].addr);
    
    mrapi_db->domains[d].nodes[n].shmems[s]=0; /* remove this node as a user */
    mrapi_db->shmems[s].refs--; /* decrement the reference count */
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(shmems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_get
    
  Description: Gets the semaphore for the given semaphore key. 
    
  Parameters: 
    
  Returns:  boolean indicating success or failure 
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_get (mrapi_sem_hndl_t* sem_hndl, 
                                      mrapi_sem_id_t key)
  {
    uint32_t s;
    mrapi_boolean_t rc = MRAPI_FALSE;
    uint32_t d = 0;
    uint32_t n = 0;
    mrapi_node_t node_id;
    mrapi_domain_t domain_id;
    
    /* lock the database (use global lock for get/create sem|rwl|mutex) */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_sem_get (&sem,0x%x /*sem key*/);",key);
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    /* now look for the semkey */
    for (s = 0; s < MRAPI_MAX_SEMS; s++) {
      if (mrapi_db->sems[s].key == key) {  
        rc = MRAPI_TRUE;
        if (mrapi_db->domains[d].nodes[n].sems[s] == 0) {   
          mrapi_db->domains[d].nodes[n].sems[s] = 1; /* log this node as a user */
          mrapi_db->sems[s].refs++; /* bump the reference count */
        }
        break;
      }
    }
    
    /* encode the handle */
    /* note: if we didn't find it, the handle will be invalid bc the indices will be max*/
    *sem_hndl = mrapi_impl_encode_hndl(s);
    
    /* unlock the database (use global lock for get/create sem|rwl|mutex) */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_create
    
  Description: Creates the semaphore for the given semaphore key. 
    
  Parameters: key - semaphore key
              members - the number of resources
              sem - the semaphore (to be filled in)                                                                                                                                                                           
  Returns:  boolean indicating success or failure  
              
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_create(mrapi_sem_hndl_t* sem,  
                                        mrapi_sem_id_t key,
                                        const mrapi_sem_attributes_t* attributes, 
                                        mrapi_uint32_t shared_lock_limit,
                                        mrapi_status_t* mrapi_status) 
  {
    uint16_t s;
  
    *mrapi_status = MRAPI_ERR_SEM_LIMIT;
    mrapi_boolean_t rc = MRAPI_FALSE;

    /* lock the database (use global lock for get/create sem|rwl|mutex)*/
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    if (mrapi_impl_create_lock_locked(sem,key,shared_lock_limit,SEM,mrapi_status)) {
      mrapi_assert (mrapi_impl_decode_hndl(*sem,&s));
      mrapi_db->sems[s].type = SEM;
      if (attributes != NULL) {
        /* set the user-specified attributes */
        mrapi_db->sems[s].attributes.ext_error_checking = attributes->ext_error_checking;
        mrapi_db->sems[s].attributes.shared_across_domains = attributes->shared_across_domains;
      } else {
        /* set the default attributes */
        mrapi_db->sems[s].attributes.ext_error_checking = MRAPI_FALSE;
        mrapi_db->sems[s].attributes.shared_across_domains = MRAPI_TRUE;
      }
      rc = MRAPI_TRUE;
    }
    /* unlock the database (use global lock for get/create sem|rwl|mutex)*/
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_init_attributes
  
  Description:
  
  Parameters:
  
  Returns:  boolean 
  
  ***************************************************************************/
  void mrapi_impl_sem_init_attributes(mrapi_sem_attributes_t* attributes) 
  {
    attributes->ext_error_checking = MRAPI_FALSE;
    attributes->shared_across_domains = MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_set_attribute
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_sem_set_attribute (mrapi_sem_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, 
                                     const void* attribute,
                                     size_t attr_size, 
                                     mrapi_status_t* status)
  {  
    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(attributes->ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->ext_error_checking,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(attributes->shared_across_domains)) {
      *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->shared_across_domains,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_get_attribute
    
  Description:
                                                                                                                                                                             
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_sem_get_attribute (mrapi_sem_hndl_t sem, 
                                     mrapi_uint_t attribute_num, 
                                     void* attribute,
                                     size_t attr_size, 
                                     mrapi_status_t* status)
  {
    uint16_t m;
    
    mrapi_assert (mrapi_impl_decode_hndl(sem,&m));

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,m,MRAPI_TRUE));
    
    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(mrapi_db->sems[m].attributes.ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[m].attributes.ext_error_checking,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(mrapi_db->sems[m].attributes.shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->sems[m].attributes.shared_across_domains,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,m));
  }
  
  /***************************************************************************
  Function: mrapi_impl_create_lock_locked
  
  Description: Creates the semaphore for the given semaphore key. 
  
  Parameters: key - semaphore key
              members - the number of resources
              sem - the semaphore (to be filled in)                                                                                                                                                                           
  Returns:  boolean indicating success or failure  
              
  ***************************************************************************/
mrapi_boolean_t mrapi_impl_create_lock_locked(mrapi_sem_hndl_t* sem,  
                                       mrapi_sem_id_t key,
                                       mrapi_uint32_t shared_lock_limit,
                                       lock_type t,
                                       mrapi_status_t* mrapi_status)
  {
    int l = 0;
    int id = 0;
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t s = 0;
    mrapi_node_t node_id;
    mrapi_domain_t d_id;
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    mrapi_dprintf(1,"mrapi_impl_sem_create (&sem,0x%x /*key*/,attrs,%d /*shared_lock_limit*/,&status);",
                  key,shared_lock_limit);
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&d_id,&d));
    
    /* make sure the semaphore doesn't already exist */
    /* Even though we checked for this at the mrapi layer, we have to check again here because
       the check and the create aren't atomic at the top layer. */
    for (s = 0; s < MRAPI_MAX_SEMS; s++) {
      /* make sure the semaphore doesn't already exist */
      /* Even though we checked for this at the mrapi layer, we have to check again here because
         the check and the create aren't atomic at the top layer. */
      if (mrapi_db->sems[s].valid && mrapi_db->sems[s].key == key) {
        if (t == RWL) { *mrapi_status = MRAPI_ERR_RWL_EXISTS; }
        else if (t == SEM) { *mrapi_status = MRAPI_ERR_SEM_EXISTS; }
        else if (t == MUTEX) { *mrapi_status = MRAPI_ERR_MUTEX_EXISTS; }
       mrapi_dprintf(1,"Unable to create mutex/sem/rwl because this key already exists key=%d",key);
        break;
      }
     }

   /* if we made it through the database without finding a semaphore with this key, then create the new semaphore */   
    if (s == MRAPI_MAX_SEMS) {
    /* update the database */
    for (s = 0; s < MRAPI_MAX_SEMS; s++) {
      if ((mrapi_db->sems[s].valid == MRAPI_FALSE) && (mrapi_db->sems[s].deleted == MRAPI_FALSE))  {
        mrapi_dprintf(1,"mrapi_impl_sem_create: Adding new semaphore set with %d shared_lock_limit (dindex=%d,nindex=%d,semindex=%d id=%d, key=%d",
                      shared_lock_limit,d,n,s,id,key);
        memset(&mrapi_db->sems[s],0,sizeof(mrapi_sem_t));
        mrapi_db->sems[s].id = id;
        mrapi_db->sems[s].key = key;
        /* log this node as a user, this way we can decrease the ref count when this node calls finalize */
        mrapi_db->domains[d].nodes[n].sems[s]=1;   
        mrapi_db->sems[s].refs++; /* bump the reference count */
        mrapi_db->sems[s].valid = MRAPI_TRUE;
        mrapi_db->sems[s].shared_lock_limit = shared_lock_limit;
        mrapi_db->sems[s].type = t;
        mrapi_db->num_sems++;
        for (l=0; l < shared_lock_limit; l++) {
          mrapi_db->sems[s].locks[l].valid = MRAPI_TRUE;
        }
        *sem = mrapi_impl_encode_hndl(s);
        mrapi_db->sems[s].handle = *sem;  
        rc = MRAPI_TRUE;
        break;
      } 
    } 
    }
 
    if (s == MRAPI_MAX_SEMS) {
      mrapi_dprintf(1,"Unable to create mutex/sem/rwl because there is no more room in the database.");
      mrapi_dprintf(1,"Suggestion, reconfigure with a larger MAX_SEM count or if deleted is a big number, disable extended error checking.\n");
    }
 
    return rc;
  }
  
  
  /***************************************************************************
  Function: mrapi_impl_sem_delete
    
  Description: remove the semaphore for the given semaphore id.
    
  Parameters: sem - semaphore
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_delete (mrapi_sem_hndl_t sem) 
  {    
    uint16_t s;
    mrapi_boolean_t rc = MRAPI_TRUE;
    uint32_t my_locks = 0;
    uint32_t l = 0;
    uint32_t n = 0;
    uint32_t d = 0;
    mca_node_t node_id;
    mca_domain_t domain_id;
    
    if (!mrapi_impl_decode_hndl(sem,&s)) {
      return MRAPI_FALSE;
    }

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_sem_delete (0x%x);",sem);
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    /* check that this node has an exclusive lock on this semaphore */
    for (l = 0; l < mrapi_db->sems[s].shared_lock_limit; l++) {
      if ((mrapi_db->sems[s].locks[l].valid) &&
          (mrapi_db->sems[s].locks[l].locked == MRAPI_TRUE) &&
          (mrapi_db->sems[s].locks[l].lock_holder_nindex == n) && 
          (mrapi_db->sems[s].locks[l].lock_holder_dindex == d)) {
        my_locks++;
      }
    }
    
    if (my_locks != mrapi_db->sems[s].shared_lock_limit) {
      mrapi_dprintf(1,"sem not exclusively locked by caller shared_lock_limit=%d, my locks=%d",
                    mrapi_db->sems[s].shared_lock_limit,my_locks);
      /* *status=MRAPI_NOT_LOCKED; */
      rc = MRAPI_FALSE;
    }
    
    if ((rc) && (mrapi_db->sems[s].valid)) {
      mrapi_db->sems[s].valid = MRAPI_FALSE;
     
      /* We do not check the reference count, it's up to the user to not delete
         semaphores that other nodes are using.  The reference count is only used
         by finalize to know if the last user of a semaphore has called finalize and
         if so, then it will delete the semaphore. */
 
      /* If extended error checking is enabled, mark it as deleted.  This will prevent
         us from overwriting this entry.  This could be a problem if semaphores are coming and
         going in the user's app because we could run out of entries.*/
      if (mrapi_db->sems[s].attributes.ext_error_checking == MRAPI_TRUE) {
        mrapi_db->sems[s].deleted = MRAPI_TRUE;
      }
      
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,s));
    return rc;
  }
  
  /***************************************************************************
  Function:mrapi_impl_yield_locked

  Description: releases the lock, attempts to yield, re-acquires the lock.

  Parameters: none

  Returns: none
  ***************************************************************************/
  void mrapi_impl_yield_locked (int which_semid,int member)
  {
    /* call this version of sched_yield when you have the lock */
    /* release the lock */
    mrapi_dprintf(4,"mrapi_impl_yield_locked");
    mrapi_assert(mrapi_impl_access_database_post(which_semid,member));
    sched_yield();
    /* re-acquire the lock */
    mrapi_assert(mrapi_impl_access_database_pre(which_semid,member,MRAPI_TRUE));
  }
  
  /***************************************************************************
  Function: mrapi_impl_sem_lock (blocking)
                                                                                                                                                                             
  Description: locks the given semaphore
  
  Parameters: sem - the semaphore 
  
  Returns:  boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_lock(mrapi_sem_hndl_t sem, 
                                      int32_t num_locks,
                                      mrapi_timeout_t timeout, 
                                      mrapi_status_t* mrapi_status)
  {
    /* blocking version */
    mrapi_timeout_t time = 0;
    uint16_t s;
    mrapi_boolean_t rc = MRAPI_TRUE;
    
    if (!mrapi_impl_decode_hndl(sem,&s)) {
      return MRAPI_FALSE;
    } 

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
    
    mrapi_dprintf(2,"mrapi_impl_sem_lock (0x%x,%d /*num_locks*/,%d /*timeout*/,&status);",
                  sem,num_locks,timeout);
    
    while (1) {
      time++;
      if (mrapi_impl_acquire_lock_locked(sem,num_locks,mrapi_status) == num_locks) {
        *mrapi_status = MRAPI_SUCCESS;
        break;
      } 
      /* try to yield */
      mrapi_dprintf(6,"mrapi_impl_sem_lock: unable to get lock, attemping to yield...");
      /* we have the semaphore, so use this version of yield  */
      mrapi_impl_yield_locked(sems_semid,0);
      
      if ((timeout !=  MRAPI_TIMEOUT_INFINITE) && (time >= timeout)) {
        if (timeout != 1) {
            /* don't overwrite ELOCKED status if this is a trylock */
          *mrapi_status = MRAPI_TIMEOUT;
        }
        rc = MRAPI_FALSE;
        break;
      }
    }
  
    /* unlock the database */
    mrapi_assert (mrapi_impl_access_database_post(sems_semid,s));
    return rc;
  }
  
  
  /***************************************************************************
  Function: mrapi_impl_acquire_lock_locked
    
  Description: This function is used by both the blocking and non-blocking 
       lock functions.
       
  Parameters: 
       
  Returns: boolean indicating success or failure
       
  ***************************************************************************/
  int32_t mrapi_impl_acquire_lock_locked(mrapi_sem_hndl_t sem, 
                                        int32_t num_locks,
                                        mrapi_status_t* status)
  {
    
    /* The database should already  be locked ! */
    
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t l = 0;
    uint16_t s = 0;
    int32_t num_added = 0;
    mca_node_t node_id;
    mca_domain_t domain_id;
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    if (!mrapi_impl_decode_hndl(sem,&s)) {
      return MRAPI_FALSE;
    }
    mrapi_dprintf(2,"mrapi_impl_acquire_lock_locked sem=%x,dindex=%d,nindex=%d,semindex=%d id=%d num_locks=%d",
                  sem,d,n,s,mrapi_db->sems[s].id,num_locks);
    mrapi_dprintf(2," (mrapi_db->sems[s].shared_lock_limit(%d) mrapi_db->sems[s].num_locks(%d) num_locks(%d))",
                  mrapi_db->sems[s].shared_lock_limit,mrapi_db->sems[s].num_locks, num_locks);
    
    if ((mrapi_db->sems[s].shared_lock_limit - mrapi_db->sems[s].num_locks - num_locks ) >= 0) {
      for (l = 0; l < mrapi_db->sems[s].shared_lock_limit; l++) {
        if ((mrapi_db->sems[s].locks[l].valid) &&
            (mrapi_db->sems[s].locks[l].locked == MRAPI_FALSE)) {
          mrapi_db->sems[s].locks[l].locked = MRAPI_TRUE;
          mrapi_db->sems[s].locks[l].lock_holder_nindex = n;
          mrapi_db->sems[s].locks[l].lock_holder_dindex = d;
          num_added++;
          mrapi_db->sems[s].num_locks++;
        }
        if (num_added == num_locks) { mrapi_dprintf(3,"got the lock(s)"); *status = MRAPI_SUCCESS; break; }
      }
    } else {
      mrapi_dprintf(3,"unable to get the lock");
      if ( mrapi_db->sems[s].type == RWL) { *status = MRAPI_ERR_RWL_LOCKED; }
      else if ( mrapi_db->sems[s].type == SEM) { *status = MRAPI_ERR_SEM_LOCKED; }
      else if ( mrapi_db->sems[s].type == MUTEX) { *status = MRAPI_ERR_MUTEX_LOCKED; }
    }
    
    return num_added;
  }
  
  /***************************************************************************
  Function: mrapi_sem_unlock
    
  Description: unlocks the given semaphore
    
  Parameters: sem - the semaphore
    
  Returns:   
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_sem_unlock(mrapi_sem_hndl_t sem,
                                        int32_t num_locks,
                                        mrapi_status_t* mrapi_status) 
  { 
    return mrapi_impl_release_lock(sem,num_locks,mrapi_status);
  }
  
  /***************************************************************************
  Function: mrapi_release_lock
    
  Description: unlocks the given semaphore
    
  Parameters: sem - the semaphore
    
  Returns:   
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_release_lock(mrapi_sem_hndl_t sem,
                                         int32_t num_locks, 
                                         mrapi_status_t* mrapi_status) 
  { 
    
    //  struct sembuf sem_unlock={ member, 1, 0};
    uint16_t s,l;
    uint32_t d = 0;
    uint32_t n = 0;
    uint32_t my_locks = 0;
    uint32_t num_removed = 0;
    mrapi_boolean_t rc = MRAPI_TRUE;
    mca_node_t node_id;
    mca_domain_t domain_id;
    
    if (!mrapi_impl_decode_hndl(sem,&s)) {
      return MRAPI_FALSE;
    }
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(sems_semid,s,MRAPI_TRUE));
    
    mrapi_dprintf(2,"mrapi_impl_sem_unlock(%x);",sem);
    
    
    /* even though these conditions were checked at the mrapi level, we have to check again
       because they could have changed (it's not atomic) */
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    if (mrapi_db->sems[s].valid == MRAPI_FALSE){
      rc = MRAPI_FALSE;
      if ( mrapi_db->sems[s].type  == RWL) { *mrapi_status = MRAPI_ERR_RWL_INVALID; }
      else if ( mrapi_db->sems[s].type == SEM) { *mrapi_status = MRAPI_ERR_SEM_INVALID; }
      else if ( mrapi_db->sems[s].type == MUTEX) { *mrapi_status = MRAPI_ERR_MUTEX_INVALID; }
    } else {
      /* check that this node has num_locks to unlock */
      for (l = 0; l < mrapi_db->sems[s].shared_lock_limit; l++) {
        if ((mrapi_db->sems[s].locks[l].valid) &&
            (mrapi_db->sems[s].locks[l].locked == MRAPI_TRUE) &&
            (mrapi_db->sems[s].locks[l].lock_holder_nindex == n) && 
            (mrapi_db->sems[s].locks[l].lock_holder_dindex == d)) {
          my_locks++;
        }
      }
      if (my_locks < num_locks) {
        rc = MRAPI_FALSE;
        if ( mrapi_db->sems[s].type == RWL) { *mrapi_status = MRAPI_ERR_RWL_NOTLOCKED; }
        else if ( mrapi_db->sems[s].type == SEM) { *mrapi_status = MRAPI_ERR_SEM_NOTLOCKED; }
        else if ( mrapi_db->sems[s].type == MUTEX) { *mrapi_status = MRAPI_ERR_MUTEX_NOTLOCKED; }
      } else {  
        /* update the lock array in our db */
        mrapi_db->sems[s].num_locks -= num_locks;
        for (l = 0; l < mrapi_db->sems[s].shared_lock_limit; l++) {
          if ((mrapi_db->sems[s].locks[l].valid) &&
              (mrapi_db->sems[s].locks[l].locked == MRAPI_TRUE) &&
              (mrapi_db->sems[s].locks[l].lock_holder_nindex == n) && 
              (mrapi_db->sems[s].locks[l].lock_holder_dindex == d)) {
            mrapi_db->sems[s].locks[l].locked = MRAPI_FALSE;
            num_removed++;
          }
        }
        mrapi_assert (num_removed == num_locks);
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(sems_semid,s));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_rmem_id
  
  Description:  
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_rmem_id (mrapi_rmem_id_t rmem_id) 
  {
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_atype
    
  Description:  
    
  Parameters: 
    
  Returns: boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_atype (mrapi_rmem_atype_t access_type) 
  {
    if (access_type == MRAPI_RMEM_DUMMY) {
      return MRAPI_TRUE;
    } else {
      return MRAPI_FALSE;
    }
  }

  /***************************************************************************
  Function: mrapi_impl_rmem_attached
    
  Description:  
    
  Parameters: 
    
  Returns: boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_attached (mrapi_rmem_hndl_t rmem) 
  {
    uint16_t r;
    uint32_t n = 0;
    uint32_t d = 0;
    mrapi_boolean_t rc = MRAPI_FALSE;
    mrapi_node_t node_id;
    mrapi_domain_t domain_id;

    /* lock the database */
   mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));

    /* make sure we recognize the caller */
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));

    if (mrapi_impl_decode_hndl(rmem,&r) && 
        mrapi_db->rmems[r].valid && 
        ( mrapi_db->rmems[r].nodes[n]==1) ) {
      rc = MRAPI_TRUE;
    }
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));

    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_rmem_exists
    
  Description:  
    
  Parameters: 
    
  Returns: boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_exists (mrapi_rmem_id_t rmem_id) 
  {
    
    mrapi_boolean_t rc = MRAPI_FALSE;
    int r;
    
    /* lock the database */
   mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_rmem_exists(%d);",rmem_id);
    
    for (r=0; r < MRAPI_MAX_RMEMS; r++) {
      if (mrapi_db->rmems[r].valid && mrapi_db->rmems[r].key == rmem_id) {
        /* *mrapi_status = MRAPI_EXISTS; */
        rc = MRAPI_TRUE;
        break;          
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_create
    
  Description:  
    
  Parameters: 
    
  Returns: boolean indicating success or failure
    
  ***************************************************************************/
  void mrapi_impl_rmem_create(mrapi_rmem_hndl_t* rmem,
                                         mrapi_rmem_id_t rmem_id,
                                         const void* mem,
                                         mrapi_rmem_atype_t access_type,
                                         const mrapi_rmem_attributes_t* attributes,
                                         mrapi_uint_t size,
                                         mrapi_status_t* status) 
  {
    
    uint32_t n = 0;
    uint32_t d = 0;
    uint32_t r = 0;
    mrapi_boolean_t rc = MRAPI_TRUE;
    mca_node_t node_id;
    mca_domain_t domain_id;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_rmem_create(&rmem,0x%x,%d,&attrs);",rmem_id,size);
   
    *status = MRAPI_SUCCESS;
 
    /* make sure we recognize the caller */
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    /* make sure we have room in the database */
    if  (mrapi_db->num_rmems == MRAPI_MAX_RMEMS){
      *status = MRAPI_ERR_MEM_LIMIT;
      rc = MRAPI_FALSE; 
    } else {
      /* make sure the remote memory doesn't already exist */
      for (r=0; r < MRAPI_MAX_RMEMS; r++) {
        if (mrapi_db->rmems[r].valid && mrapi_db->rmems[r].key == rmem_id) {
          *status = MRAPI_ERR_RMEM_EXISTS; 
          rc = MRAPI_FALSE;
          break;          
        }
      }
    }
    
    if (rc) {
      /* update our database */
      for (r = 0; r < MRAPI_MAX_RMEMS; r++) {
        if (mrapi_db->rmems[r].valid == MRAPI_FALSE) {
          mrapi_dprintf(1,"adding rmem: id=%u to dindex=%u nindex=%u sindex=%u \n",
                        rmem_id,d,n,r);
          mrapi_db->rmems[r].key = rmem_id; /* the shared key passed in on get/create */
          mrapi_db->rmems[r].valid = MRAPI_TRUE;
          mrapi_db->rmems[r].addr = mem;
          mrapi_db->rmems[r].size = size;
          mrapi_db->rmems[r].access_type = access_type;
          /* fill in the attributes */
          if (attributes != NULL) {
            /* set the user-specified attributes */
            mrapi_db->rmems[r].attributes.ext_error_checking = attributes->ext_error_checking;
            mrapi_db->rmems[r].attributes.shared_across_domains = attributes->shared_across_domains;
          } else {
            /* set the default attributes */
            mrapi_db->rmems[r].attributes.ext_error_checking = MRAPI_FALSE;
            mrapi_db->rmems[r].attributes.shared_across_domains = MRAPI_TRUE;
          }
          mrapi_db->num_rmems++;
          break;
        }
      }
    }
    
    if (!rc) {
      r = MRAPI_MAX_RMEMS;
    }
    
    *rmem = mrapi_impl_encode_hndl(r);
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_init_attributes
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
                                                                                                                                                                             
  ***************************************************************************/
  void mrapi_impl_rmem_init_attributes(mrapi_rmem_attributes_t* attributes) 
  {
    attributes->ext_error_checking = MRAPI_FALSE;
    attributes->shared_across_domains = MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_set_attribute
    
  Description:
    
  Parameters:
    
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_rmem_set_attribute (mrapi_rmem_attributes_t* attributes, 
                                      mrapi_uint_t attribute_num, 
                                      const void* attribute,
                                      size_t attr_size, 
                                      mrapi_status_t* status)
  {  

    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(attributes->ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->ext_error_checking,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(attributes->shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy(&attributes->shared_across_domains,attribute,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };

  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_get_attribute
    
  Description:
    
  Parameters:
                                                                                                                                                                           
  Returns:  boolean 
  ***********************************************************************/
  void mrapi_impl_rmem_get_attribute (mrapi_rmem_hndl_t rmem, 
                                      mrapi_uint_t attribute_num, 
                                      void* attribute,
                                      size_t attr_size, 
                                      mrapi_status_t* status)
  {
    uint16_t r;
    
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));

    switch(attribute_num) {
    case (MRAPI_ERROR_EXT): 
      if (attr_size != sizeof(mrapi_db->rmems[r].attributes.ext_error_checking)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->rmems[r].attributes.ext_error_checking,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    case (MRAPI_DOMAIN_SHARED): 
      if (attr_size != sizeof(mrapi_db->rmems[r].attributes.shared_across_domains)) {
        *status = MRAPI_ERR_ATTR_SIZE;
      } else {
        memcpy((mrapi_boolean_t*)attribute,&mrapi_db->rmems[r].attributes.shared_across_domains,attr_size);
        *status = MRAPI_SUCCESS;
      }
      break;
    default:
      *status = MRAPI_ERR_ATTR_NUM;
    };
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_get
    
  Description:
    
  Parameters: 
    
  Returns: 
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_get(mrapi_rmem_hndl_t* rmem_hndl,uint32_t rmem_id) 
  {
    mrapi_boolean_t rc = MRAPI_FALSE;
    int r;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    mrapi_dprintf(1,"mrapi_impl_rmem_get (&rmem,0x%x /*rmem_id*/);",rmem_id);
    
    /* now look for the shared key (aka rmem_id) */
    for (r = 0; r < MRAPI_MAX_RMEMS; r++) {
      if (mrapi_db->rmems[r].key == rmem_id) {  
        rc = MRAPI_TRUE;
        break;
      }
    }
    
    /* encode the handle */
    /* note: if we didn't find it, the handle will be invalid bc the indices will be max*/
    *rmem_hndl = mrapi_impl_encode_hndl(r);
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_rmem_hndl

  Description: Checks if the sem handle refers to a valid rmem segment

  Parameters:
  
  Returns: true/false indicating success or failure
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_rmem_hndl(mrapi_rmem_hndl_t rmem) 
  {
    uint16_t r;
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    if (mrapi_impl_decode_hndl(rmem,&r) && 
        (r < MRAPI_MAX_RMEMS) && 
        mrapi_db->rmems[r].valid) {
      rc = MRAPI_TRUE;
    }
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_attach
    
  Description: 
                                                                                                                                                                             
  Parameters: 
    
  Returns: 
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_attach (mrapi_rmem_hndl_t rmem) 
  {
    uint16_t r;
    uint32_t n = 0;
    uint32_t d = 0;
    mrapi_node_t node_id;
    mrapi_domain_t d_id;

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&d_id,&d));
    mrapi_dprintf(1,"mrapi_impl_rmem_attach(0x%x);",rmem);
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    
    if ( mrapi_db->rmems[r].nodes[n]==0) {
      mrapi_db->rmems[r].nodes[n]=1; /* log this node as a user */
      mrapi_db->rmems[r].refs++; /* bump the reference count */
    }
    
   /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));

    
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_detach
    
  Description: 
    
  Parameters: 
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_detach(mrapi_rmem_hndl_t rmem) 
  {  
    uint16_t r;
    uint32_t d = 0;
    uint32_t n = 0;
    mrapi_domain_t domain_id;
    mrapi_node_t node_id;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert(mrapi_impl_whoami(&node_id,&n,&domain_id,&d));
    
    mrapi_dprintf(1,"mrapi_impl_rmem_detach(handle=0x%x);",rmem);
    
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    
    mrapi_db->rmems[r].nodes[n]=0; /* remove this node as a user */
    mrapi_db->rmems[r].refs--; /* decrement the reference count */
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));

    return MRAPI_TRUE;  
  }
  
  /***************************************************************************
  Function: mrapi_delete_sharedMem
    
  Description: 
    
  Parameters: shmem_address - the address of the shared memory segment
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_delete(mrapi_rmem_hndl_t rmem) 
  {  
    uint16_t r;
    mrapi_boolean_t rc = MRAPI_TRUE;
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    
    mrapi_dprintf(1,"mrapi_impl_rmem_delete(0x%x);",rmem);
    
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    
    mrapi_db->rmems[r].valid = MRAPI_FALSE;
    
    /* FIXME: WHAT IF OTHERS ARE STILL ATTACHED?*/
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    
    return rc;
  }
  
    /***************************************************************************
  Function: mrapi_impl_rmem_read_i
    
  Description: 
    
  Parameters: 
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_read_i( mrapi_rmem_hndl_t rmem,
                                          mrapi_uint32_t rmem_offset,
                                          void* local_buf,
                                          mrapi_uint32_t local_offset,
                                          mrapi_uint32_t bytes_per_access,
                                          mrapi_uint32_t num_strides,
                                          mrapi_uint32_t rmem_stride,
                                          mrapi_uint32_t local_stride,
                                          mrapi_status_t* status,
                                          mrapi_request_t* request)
  {
    
    mrapi_boolean_t rc = MRAPI_FALSE;
    
    uint32_t r = mrapi_impl_setup_request();
    char status_buff[MRAPI_MAX_STATUS_SIZE];
    if (r < MRAPI_MAX_REQUESTS) {
    rc |= mrapi_impl_rmem_read(rmem,rmem_offset,local_buf,local_offset,bytes_per_access,
                               num_strides,rmem_stride,local_stride,status);
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    // update the request
    mrapi_db->requests[r].completed = MRAPI_TRUE;
    mrapi_db->requests[r].status = *status;
    *status = MRAPI_SUCCESS; // we were successful setting up the request
    *request = mrapi_impl_encode_hndl(r);
    mrapi_dprintf(3,"mrapi_impl_rmem_read_i r=%d request=0x%x completed=%d status=%s",
                  r,
                  *request,
                  mrapi_db->requests[r].completed,
                  mrapi_display_status(mrapi_db->requests[r].status,status_buff,sizeof(status_buff))); 
    /* unlock the database */
   mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));

  } else {
     *request = 0xffffffff;
     *status = MRAPI_ERR_REQUEST_LIMIT; 
      mrapi_dprintf(3,"mrapi_impl_rmem_read_i returning MRAPI_ERR_REQUEST_LIMIT");
  }
    
    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_rmem_read
    
  Description: 
    
  Parameters: 
    
  Returns:  boolean indicating success or failure
    
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_read( mrapi_rmem_hndl_t rmem,
                                        mrapi_uint32_t rmem_offset,
                                        void* local_buf,
                                        mrapi_uint32_t local_offset,
                                        mrapi_uint32_t bytes_per_access,
                                        mrapi_uint32_t num_strides,
                                        mrapi_uint32_t rmem_stride,
                                        mrapi_uint32_t local_stride,
                                        mrapi_status_t* status)
  {
    uint16_t r,i;
    mrapi_boolean_t rc = MRAPI_TRUE;
    char* rmem_buf;
    const char* last_remote_access;
    const char* upper_bound;
    
    mrapi_dprintf(1,"mrapi_impl_rmem_read handle=(0x%x) offset=(0x%x)",
                  rmem,rmem_offset); 
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    
    last_remote_access = mrapi_db->rmems[r].addr + rmem_offset + (rmem_stride * num_strides );
    upper_bound = mrapi_db->rmems[r].addr +  mrapi_db->rmems[r].size - 1;
    
    if (mrapi_db->rmems[r].valid == MRAPI_FALSE) {
      rc = MRAPI_FALSE;
    } else if ( last_remote_access >  upper_bound) {
      // buffer overrun
      rc = MRAPI_FALSE;
    } else {
      local_buf += local_offset;
      rmem_buf = (void*)mrapi_db->rmems[r].addr + rmem_offset;
      for (i = 0; i < num_strides; i++) {
        memcpy (local_buf,rmem_buf,bytes_per_access);
        rmem_buf += rmem_stride;
        local_buf += local_stride;
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    
    return rc;                             
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_write_i

  Description: 
  
  Parameters:
  
  Returns:  boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_write_i( mrapi_rmem_hndl_t rmem,
                                           mrapi_uint32_t rmem_offset,
                                           const void* local_buf,
                                           mrapi_uint32_t local_offset,
                                           mrapi_uint32_t bytes_per_access,
                                           mrapi_uint32_t num_strides,
                                           mrapi_uint32_t rmem_stride,
                                           mrapi_uint32_t local_stride,
                                           mrapi_status_t* status,
                                           mrapi_request_t* request)
  {
    
    
    mrapi_boolean_t rc = MRAPI_FALSE;
    char status_buff [MRAPI_MAX_STATUS_SIZE];
    uint32_t r = mrapi_impl_setup_request();
    
    if (r < MRAPI_MAX_REQUESTS) {
      rc |= mrapi_impl_rmem_write(rmem,rmem_offset,local_buf,local_offset,bytes_per_access,
                                  num_strides,rmem_stride,local_stride,status);
      /* lock the database */
      mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
      // update the request
      mrapi_db->requests[r].completed = MRAPI_TRUE;
      mrapi_db->requests[r].status = *status;
      *status = MRAPI_SUCCESS; // we were successful setting up the request
      *request = mrapi_impl_encode_hndl(r);
      mrapi_dprintf(3,"mrapi_impl_rmem_write_i r=%d request=0x%x completed=%d status=%s",
                    r,
                    *request,
                    mrapi_db->requests[r].completed,
                    mrapi_display_status(mrapi_db->requests[r].status,status_buff,sizeof(status_buff)));
      /* unlock the database */
      mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
      
    } else {
      *request = 0xffffffff;
      *status = MRAPI_ERR_REQUEST_LIMIT;
    }
    
    return rc;
  }
  
  /***************************************************************************
  Function: mrapi_impl_rmem_write

  Description: 
  
  Parameters:
  
  Returns:  boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_rmem_write( mrapi_rmem_hndl_t rmem,
                                         mrapi_uint32_t rmem_offset,
                                         const void* local_buf,
                                         mrapi_uint32_t local_offset,
                                         mrapi_uint32_t bytes_per_access,
                                         mrapi_uint32_t num_strides,
                                         mrapi_uint32_t rmem_stride,
                                         mrapi_uint32_t local_stride,
                                         mrapi_status_t* status)
  {
    
    uint16_t r,i;
    mrapi_boolean_t rc = MRAPI_TRUE;
    char* rmem_buf;
    const char* last_remote_access;
    const char* upper_bound;
    
    
    mrapi_dprintf(1,"mrapi_impl_rmem_write handle=(0x%x) offset=(0x%x)",
                  rmem,rmem_offset);  
    
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(rmems_semid,0,MRAPI_TRUE));
    
    mrapi_assert (mrapi_impl_decode_hndl(rmem,&r));
    last_remote_access = mrapi_db->rmems[r].addr + rmem_offset + (rmem_stride * num_strides ) + bytes_per_access;
    upper_bound = mrapi_db->rmems[r].addr +  mrapi_db->rmems[r].size;
    
    if (mrapi_db->rmems[r].valid == MRAPI_FALSE) {
      rc = MRAPI_FALSE;
    } else if ( last_remote_access >  upper_bound) {
      // buffer overrun
      rc = MRAPI_FALSE;
    } else {
      local_buf += local_offset;
      rmem_buf = (void*)mrapi_db->rmems[r].addr + rmem_offset;
      for (i = 0; i < num_strides; i++) {
        memcpy (rmem_buf,local_buf,bytes_per_access);
        rmem_buf += rmem_stride;
        local_buf += local_stride;
      }
    }
    
    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(rmems_semid,0));
    
    return rc;  
  }
  
  /***************************************************************************
  Function: mrapi_impl_resources_get

  Description: An implementation of getting a resource tree

  Parameters:  

  Returns:  

  ***************************************************************************/
  mrapi_resource_t* mrapi_impl_resources_get(
                                             mrapi_rsrc_filter_t subsystem_filter,
                                             mrapi_status_t* status)
  {
    uint16_t number_of_nodes = 0;
    mrapi_resource_type rsrc_type;
    mrapi_resource_t *filtered_tree = NULL;
    uint16_t number_of_filtered_children;
    
    if (subsystem_filter == MRAPI_RSRC_MEM) {
      rsrc_type = MEM;
    } else if (subsystem_filter == MRAPI_RSRC_CPU) {
      rsrc_type = CPU;
    } else if (subsystem_filter == MRAPI_RSRC_CACHE) {
      rsrc_type = CACHE;
    } else if (subsystem_filter == MRAPI_RSRC_CROSSBAR) {
      rsrc_type = CROSSBAR;
    } else if (subsystem_filter == MRAPI_RSRC_DMA) {
      *status = MRAPI_SUCCESS;
      return MRAPI_NULL;
    } else {
      *status = MRAPI_ERR_RSRC_INVALID_SUBSYSTEM;
      return MRAPI_NULL;
    }
    
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_resources_get");
    number_of_nodes = mrapi_impl_get_number_of_nodes(rsrc_type, resource_root);
    if (number_of_nodes != 0) {
      char *tree_name = "filtered tree";
      int name_length = strlen(tree_name);
      filtered_tree = (mrapi_resource_t *) malloc(sizeof(mrapi_resource_t));
      filtered_tree->name = (char *) malloc((name_length+1) * sizeof(char));
      strcpy(filtered_tree->name, tree_name);
      filtered_tree->resource_type = SYSTEM;
      filtered_tree->number_of_children = number_of_nodes;
      filtered_tree->children = (mrapi_resource_t **) malloc(number_of_nodes * sizeof(mrapi_resource_t*));
      filtered_tree->number_of_attributes = 0;
      filtered_tree->attribute_types = NULL;
      filtered_tree->attribute_values = NULL;
      
      /* Populate the filtered tree */
      number_of_filtered_children = mrapi_impl_create_rsrc_tree(rsrc_type,
								resource_root,
								0,
								filtered_tree);
    }

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return filtered_tree;
  }
  
  /*-------------------------------------------------------------------
    Search the resource tree to find the number of nodes of a certain
    resource type.
    -------------------------------------------------------------------*/
  uint16_t mrapi_impl_get_number_of_nodes(mrapi_resource_type rsrc_type,
                                          mrapi_resource_t *tree) {
    uint16_t number_of_nodes = 0;
    uint16_t number_of_children;
    uint16_t i;
    
    mrapi_dprintf(1,"mrapi_impl_get_number_of_nodes"); 
    number_of_children = tree->number_of_children;
    
    if (number_of_children == 0) {
      if (rsrc_type == tree->resource_type) {
        number_of_nodes++;
      }
      
    } else {
      for (i = 0; i < number_of_children; i++) {
        number_of_nodes += mrapi_impl_get_number_of_nodes(rsrc_type, tree->children[i]);
      }
    }

    return number_of_nodes;
  }
  
  /*-------------------------------------------------------------------
    Search the resource tree to find the number of nodes of a certain
    resource type.
    -------------------------------------------------------------------*/
  uint16_t mrapi_impl_create_rsrc_tree(mrapi_resource_type rsrc_type,
                                       mrapi_resource_t *src_tree,
                                       uint16_t start_index_child,
                                       mrapi_resource_t *filtered_tree) {
    uint16_t index_child;
    uint16_t i;
    uint32_t number_of_children;
    uint32_t number_of_attrs;
    mrapi_resource_t *the_child;
    mrapi_resource_t *new_child;
    char *new_name;
    int name_length;
    void *src = NULL;
    void *dest = NULL;
    rsrc_type_t the_type;
    rsrc_type_t *new_type;
    mrapi_attribute_static the_static;
    mrapi_attribute_static *new_static;
    mrapi_boolean_t the_start;
    mrapi_boolean_t *new_start;
    
    mrapi_dprintf(1,"mrapi_impl_create_rsrc_tree"); 
    /* Check if this node is one we're interested in */
    index_child = start_index_child;
    if (rsrc_type == src_tree->resource_type) {
      new_child = (mrapi_resource_t *) malloc(sizeof(mrapi_resource_t));
      /* Make a copy of the node, leaving out the pointers to the children */
      name_length = strlen(src_tree->name);
      new_name = (char *) malloc((name_length+1) * sizeof(char));
      strcpy(new_name, src_tree->name);
      new_child->name = new_name;
      new_child->resource_type = src_tree->resource_type;
      new_child->number_of_children = 0;
      new_child->children = NULL;
      number_of_attrs = src_tree->number_of_attributes;
      new_child->number_of_attributes = number_of_attrs;
      new_child->attribute_values = (void **) malloc(number_of_attrs * sizeof(void*));
      new_child->attribute_types  = (void **) malloc(number_of_attrs * sizeof(void*));
      new_child->attribute_static =
        (mrapi_attribute_static **) malloc(number_of_attrs * sizeof(mrapi_attribute_static *));
      new_child->attribute_started =
        (mrapi_boolean_t **) malloc(number_of_attrs * sizeof(mrapi_boolean_t *));
      for (i = 0; i < number_of_attrs; i++) {
        the_type = *((rsrc_type_t *)src_tree->attribute_types[i]);
        src = src_tree->attribute_values[i];
        if (the_type == RSRC_UINT16_T) {
          dest = (void *)malloc(sizeof(uint16_t));
          *((uint16_t*)dest) = *((uint16_t*)src);
          
        } else if (the_type == RSRC_UINT32_T) {
          dest = (void *)malloc(sizeof(uint32_t));
          *((uint32_t*)dest) = *((uint32_t*)src);
          
        } else {
          /* Should not reach here */
          mrapi_dprintf(1, "Bad resource type while copying a resource node\n");
        }
        new_child->attribute_values[i] = dest;
        
        /* Copy the types */
        new_type = (rsrc_type_t *)malloc(sizeof(rsrc_type_t));
        *new_type = the_type;
        new_child->attribute_types[i] = new_type;

        /* Copy the statics */
        the_static = *(src_tree->attribute_static[i]);
        new_static = (mrapi_attribute_static *)malloc(sizeof(mrapi_attribute_static));
        *new_static = the_static;
        new_child->attribute_static[i] = new_static;
        
        /* Copy the starts */
        the_start = *(src_tree->attribute_started[i]);
        new_start = (mrapi_boolean_t *)malloc(sizeof(mrapi_boolean_t));
        *new_start = the_start;
        new_child->attribute_started[i] = new_start;
      }
      
      /* Add this to the filter tree */
      filtered_tree->children[index_child] = new_child;
      
      /* Increment the index so that the next addition to the tree is to */
      /* the correct child */
      index_child++;
    }

    /* Check the children */
    number_of_children = src_tree->number_of_children;
    if (number_of_children != 0) {
      /* Recursively check the children to see if they are type we're interested in */
      for (i = 0; i < number_of_children; i++) {
        the_child = src_tree->children[i];
        index_child = mrapi_impl_create_rsrc_tree(rsrc_type,
						  the_child,
						  index_child,
						  filtered_tree);
      }
    }

    return index_child;
  }

  /***************************************************************************
  Function: mrapi_impl_resource_tree_free

  Description:  Frees the memory of a filtered resource tree.

  Parameters:  A pointer to the tree

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_resource_tree_free(
                                                mrapi_resource_t* const * root_ptr,
                                                mrapi_status_t* status)
  {
    uint16_t number_of_children;
    uint16_t number_of_attributes;
    uint16_t i;
    mrapi_resource_t *the_child;
    mrapi_resource_t *root = *root_ptr;  /* FIXME: COMPILE WARNING RE CONST */
    mrapi_boolean_t rc = MRAPI_TRUE;

    /* Check for invalid resource trees */
    if (root == NULL) {
      *status = MRAPI_ERR_RSRC_INVALID_TREE;
      rc = MRAPI_FALSE;
      return rc;
    }

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_resource_tree_free");
    number_of_children = root->number_of_children;
    for (i = 0; i < number_of_children; i++) {
      the_child = root->children[i];
      free(the_child->name);
      free(the_child);
    }
    free(root->children);
    free(root->name);
    number_of_attributes = root->number_of_attributes;
    for (i = 0; i < number_of_attributes; i++) {
      free(root->attribute_values[i]);
    }
    free(root->attribute_values);
    free(root);

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_resource_get_attribute

  Description:  Frees the memory of a filtered resource tree.

  Parameters:  A pointer to the tree

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_resource_get_attribute(
                                                    const mrapi_resource_t* resource,
                                                    mrapi_uint_t attribute_number,
                                                    void* attribute_value,
                                                    size_t attr_size,
                                                    mrapi_status_t* status)
  {
    rsrc_type_t the_type;
    mrapi_boolean_t rc = MRAPI_TRUE;
  
    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_resource_get_attribute");
    /* Not all possible combinations are implemented in this example */
    the_type = *((rsrc_type_t *)resource->attribute_types[attribute_number]);
    if (the_type == RSRC_UINT16_T) {
      uint16_t val = *((uint16_t *)resource->attribute_values[attribute_number]);
      *((uint16_t*)attribute_value) = val;
    } else if (the_type == RSRC_UINT32_T) {
      uint32_t val = *((uint32_t *)resource->attribute_values[attribute_number]);
      *((uint32_t*)attribute_value) = val;
    }

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /*-------------------------------------------------------------------
    A backdoor to artificially cause a dynamic attribute to change
    Current only works for attribute number 1.
    -------------------------------------------------------------------*/
  void mrapi_impl_increment_cache_hits(mrapi_resource_t *resource, int increment)
  {
    int attribute_number = 1;  /* Good only for L3cache number of cache hits */
    *((uint32_t*)resource->attribute_values[attribute_number]) += increment;
  }

  /***************************************************************************
  Function: mrapi_impl_dynamic_attribute_reset

  Description:  Resets a dynamic attribute

  Parameters:  A pointer to the resource

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_dynamic_attribute_reset(
                                                     const mrapi_resource_t *resource,
                                                     mrapi_uint_t attribute_number,
                                                     mrapi_status_t* status)
  {
    rsrc_type_t the_type;
    mrapi_boolean_t rc = MRAPI_TRUE;

    /* lock the database */
    mrapi_assert (mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_dynamic_attribute_reset");
    the_type = *((rsrc_type_t *)resource->attribute_types[attribute_number]);
    if (the_type == RSRC_UINT16_T) {
      *((uint16_t*)resource->attribute_values[attribute_number]) = 0;
    } else if (the_type == RSRC_UINT32_T) {
      *((uint32_t*)resource->attribute_values[attribute_number]) = 0;
    }

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_dynamic_attribute_start

  Description:  Resets a dynamic attribute

  Parameters:  A pointer to the resource

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_dynamic_attribute_start(
                                                     const mrapi_resource_t* resource,
                                                     mrapi_uint_t attribute_number,
                                                     void (*rollover_callback) (void),
                                                     mrapi_status_t* status)
  {
    mrapi_boolean_t rc = MRAPI_TRUE;

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_dynamic_attribute_start");
    if (strcmp(resource->name, "L3 Cache") == 0) {
      /* Since there are no hooks to underlying hardware in this example, we will fake
         L3 cache hits.  Here, we only change the attribute to started.  On
         real hardware, one might program performance monitors to gather
         L3 cache hits. */
      if (attribute_number == 1) {
        *(resource->attribute_started[attribute_number]) = MRAPI_TRUE;
        mrapi_db->rollover_callbacks_ptr[mrapi_db->rollover_index] = rollover_callback;
        mrapi_db->rollover_index++;
      }
    }

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_dynamic_attribute_stop

  Description:  Resets a dynamic attribute

  Parameters:  A pointer to the resource

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_dynamic_attribute_stop(
                                                    const mrapi_resource_t* resource,
                                                    mrapi_uint_t attribute_number,
                                                    mrapi_status_t* status)
  {
    mrapi_boolean_t rc = MRAPI_TRUE;

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    mrapi_dprintf(1,"mrapi_impl_dynamic_attribute_stop");
    if (strcmp(resource->name, "L3 Cache") == 0) {
      /* Since there are no hooks to underlying hardware in this example, we will fake
         L3 cache hits.  Here, we only change the attribute to started.  On
         real hardware, one might program performance monitors to gather
         L3 cache hits. */
      if (attribute_number == 1) {
        *(resource->attribute_started[attribute_number]) = MRAPI_FALSE;
      }
    }

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /***************************************************************************
  Function: mrapi_impl_resource_register_callback

  Description:  

  Parameters:  

  Returns:  Nothing

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_resource_register_callback(
                                                        mrapi_event_t event,
                                                        unsigned int frequency,
                                                        void (*callback_function) (mrapi_event_t event),
                                                        mrapi_status_t* status)
  {
    mrapi_boolean_t rc = MRAPI_TRUE;
    mrapi_node_t node_id;
    mrapi_uint16_t index;

    if (event == (mrapi_event_t)NULL) {
      *status = MRAPI_ERR_RSRC_INVALID_EVENT;
      return MRAPI_FALSE;
    }

    node_id = mrapi_node_id_get(status);
    if (*status != MRAPI_SUCCESS) {
      return MRAPI_FALSE;
    }

    /* lock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));

    index = mrapi_db->callback_index;
    mrapi_dprintf(1,"mrapi_impl_resource_register_callback, index %d, node id %d",
		  index, node_id);
    (mrapi_db->callbacks_array[index]).callback_func      = callback_function;
    (mrapi_db->callbacks_array[index]).callback_event     = event;
    (mrapi_db->callbacks_array[index]).callback_frequency = frequency;
    (mrapi_db->callbacks_array[index]).callback_count     = 0;
    (mrapi_db->callbacks_array[index]).node_id            = node_id;
    mrapi_db->callback_index = index + 1;
    mrapi_dprintf(1,"mrapi_impl_resource_register_callback, callback index %d",
		  mrapi_db->callback_index);

    /* Set up an alarm that will artificially cause the event to happen */
    sigemptyset(&alarm_struct.sa_mask);
    alarm_struct.sa_flags = 0;
    alarm_struct.sa_handler = mrapi_impl_alarm_catcher;
    sigaction(SIGALRM, &alarm_struct, NULL);
    /* Set the alarm for 1 second) */
    alarm(1);

    /* unlock the database */
    mrapi_assert(mrapi_impl_access_database_post(semid,0));

    *status = MRAPI_SUCCESS;
    return rc;
  }

  /***************************************************************************
  Have the mrapi_impl_alarm_catcher artificially call the function that causes the
  event to occur.
  ***************************************************************************/
  void mrapi_impl_alarm_catcher(int signal) {
    mrapi_impl_cause_event();
  }
  
  /***************************************************************************
   * Because the alarms are not tied to a specific node in this example,
   * it is possible that any one of the nodes will actually get the callback
   * invoked.
   ***************************************************************************/
  void mrapi_impl_cause_event() {
    mrapi_event_t the_event;
    mrapi_node_t node_id;
    mrapi_node_t current_node_id;
    int i;
    int index = -1;
    int max_index = 0;
    mrapi_status_t status;

    node_id = mrapi_node_id_get(&status);
    mrapi_dprintf(4, "mrapi_impl_cause_event current node %d", node_id);

    /* Lock the database, increment the callback count, and unlock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));
    max_index = mrapi_db->callback_index;
    /* Find a callback with the same node id */
    for (i = 0; i < max_index && index == -1; i++) {
      current_node_id = (mrapi_db->callbacks_array[i]).node_id;
      /*       mrapi_dprintf(4, "checking index %d with node id %d", i, current_node_id); */
      if (current_node_id == node_id) {
        index = i;
      }
    }
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    
    if (index > -1 && index < max_index) {
      mrapi_dprintf(4, "found callback at index %d", index);
      
    } else {
      sigaction(SIGALRM, &alarm_struct, NULL);
      /* Set the alarm for 1 second) */
      alarm(1);
      mrapi_dprintf(4,"mrapi_impl_cause_event reseting alarm, no callback found");
      return;
    }
    
    /* Lock the database, increment the callback count, and unlock the database */
    mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));
    (mrapi_db->callbacks_array[index]).callback_count++;
    mrapi_assert(mrapi_impl_access_database_post(semid,0));
    
    /* Since this function artifically cause an event, a callback needs to be */
    /* selected.  Arbitrarily pick the first callback. */
    /* Check if it is time to invoke the callback */
    if ((mrapi_db->callbacks_array[index]).callback_count >=
        (mrapi_db->callbacks_array[index]).callback_frequency) {
      /* The frequency count has been met, so invoke the callback */
      mrapi_assert(mrapi_impl_access_database_pre(semid,0,MRAPI_TRUE));
      (mrapi_db->callbacks_array[index]).callback_count = 0;
      the_event = (mrapi_db->callbacks_array[index]).callback_event;
      mrapi_assert(mrapi_impl_access_database_post(semid,0));
      mrapi_dprintf(4, "mrapi_impl_cause_event calling callback, index %d", index);
      (*(mrapi_db->callbacks_array[index]).callback_func)(the_event);
    }
    
    /* The frequency count has not been met, so reschedule the event */
    sigaction(SIGALRM, &alarm_struct, NULL);
    /* Set the alarm for 1 second) */
    alarm(1);
    mrapi_dprintf(4,"mrapi_impl_cause_event reseting alarm, index %d", index);
  }
  
  /***************************************************************************
   ***************************************************************************/
  void mrapi_impl_trigger_rollover(uint16_t index) {
    (*(mrapi_db->rollover_callbacks_ptr[index]))();
  }
  
  /***************************************************************************
  Function: mrapi_impl_valid_attribute_number

  Description:  Checks if this is a valid attribute number

  Parameters:  

  Returns:  mrapi_boolean_t

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_valid_attribute_number(const mrapi_resource_t* resource,
						    const mrapi_uint_t attribute_number)
  {
    uint32_t number_of_attributes = resource->number_of_attributes;

    if (attribute_number >= number_of_attributes) {
      return MRAPI_FALSE;
    } else if (attribute_number < 0) {
      return MRAPI_FALSE;
    } else {
      return MRAPI_TRUE;
    }
  }

  /***************************************************************************
  Function: mrapi_impl_is_static

  Description:  Checks to see if the attribute is static

  Parameters:  

  Returns:  mrapi_boolean_t

  ***************************************************************************/
  mrapi_boolean_t mrapi_impl_is_static(const mrapi_resource_t* resource,
                                       const mrapi_uint_t attribute_number) {
    mrapi_attribute_static attr_static = *(resource->attribute_static[attribute_number]);
    if (attr_static == MRAPI_ATTR_STATIC) {
      return MRAPI_TRUE;
    } else {
      return MRAPI_FALSE;
    }
  }
  
#ifdef __cplusplus
  extern } 
#endif /* __cplusplus */
