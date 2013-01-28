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


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mcapi.h>
#include <mrapi.h>
#include <mcapi_impl_spec.h>
#include <transport_sm.h>
#include <stdio.h>
#include <unistd.h> /*for sleep*/
#include <sys/ipc.h> /* for ftok */


  /* the shared memory address */
  mrapi_shmem_hndl_t shm;
  void* shm_addr;
  
  /* These functions serve as a wrapper for the resource API (in this case MRAPI). The
     function prototypes are defined in transport_sm.h */
  
  /* To better understand the layers of the software architecture, take a look at
     the design document in the docs directory. */


  /***************************************************************************
  NAME: transport_sm_initialize
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t transport_sm_initialize(mcapi_domain_t domain_id,
                                          mcapi_node_t node_id,
                                          uint32_t* lock_handle) 
  {
    mcapi_status_t status;
    mrapi_info_t mrapi_version; 
    mrapi_parameters_t parms = 0;
    int key = 0;
    mcapi_boolean_t rc = MCAPI_FALSE;
    char status_buff[MRAPI_MAX_STATUS_SIZE];
    /* We are using mrapi reader/writer locks */
    /* Eventually move to one lock per endpoint, for now we are just using one 
       global lock */
    uint32_t num_readers = 1;
    mrapi_initialize(domain_id,node_id,parms,&mrapi_version,&status);
    if ((status == MRAPI_SUCCESS) || (status == MRAPI_ERR_NODE_INITIALIZED))  {
      /* create  */
      key = ftok("/dev/null",'c'); 
      /* create it (it may already exist) */
      if (transport_sm_create_rwl(key,lock_handle,num_readers)) {
        rc = MCAPI_TRUE;
      } 
    } else {
      fprintf(stderr,"mrapi_initialize failed status=%s\n",
              mrapi_display_status(status,status_buff,sizeof(status_buff)));
    }
    return rc;
  }

  /***************************************************************************
  NAME: transport_sm_finalize
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t transport_sm_finalize(mcapi_boolean_t last_man_standing, 
                                        mcapi_boolean_t last_man_standing_for_this_process,
                                        mcapi_boolean_t finalize_mrapi,
                                        uint32_t handle)
  {
    mcapi_status_t status;    
    mcapi_boolean_t rc = MCAPI_TRUE;
    char status_buff[MRAPI_MAX_STATUS_SIZE];
    if (last_man_standing_for_this_process) {
      mrapi_dprintf(1,"mcapi transport_sm_finalize: detaching from shared memory\n");
      mrapi_shmem_detach(shm,&status);
      if (status != MRAPI_SUCCESS) {
        fprintf(stderr,"ERROR: transport_sm_finalize mrapi_shmem_detach(shm handle=%08lx, status=%s) failed\n",
                (unsigned long)shm,mrapi_display_status(status,status_buff,sizeof(status_buff)));
        rc = MCAPI_FALSE;
      }
    }
    
    if (last_man_standing) {
      mrapi_dprintf(1,"mcapi transport_sm_finalize: freeing lock and shared memory\n");
      mrapi_shmem_delete(shm,&status);
      if (status != MRAPI_SUCCESS) {
        fprintf(stderr,"ERROR: transport_sm_finalize mrapi_shmem_free(shm handle=%x) failed %s\n",
                (unsigned)shm,mrapi_display_status(status,status_buff,sizeof(status_buff)));  
        rc = MCAPI_FALSE;
      }
      mrapi_rwl_lock (handle,MCAPI_TRUE/*exclusive*/,MRAPI_TIMEOUT_INFINITE /*timeout*/,&status); 
      if (status != MRAPI_SUCCESS) {
        fprintf(stderr,"ERROR: transport_sm_finalize mrapi_rwl_lock failed  %s\n",
                mrapi_display_status(status,status_buff,sizeof(status_buff)));
        rc = MCAPI_FALSE;
      } else {
        mrapi_rwl_delete(handle,&status);
        if (status != MRAPI_SUCCESS) {
          fprintf(stderr,"ERROR: transport_sm_finalize mrapi_rwl_free(handle=%x ) failed %s\n",
                  handle,mrapi_display_status(status,status_buff,sizeof(status_buff)));
          rc = MCAPI_FALSE;
        }
      }
    } 
    if (finalize_mrapi) {
      /* tell mrapi to finalize this node */
      mrapi_finalize (&status);
      if (status != MRAPI_SUCCESS) {
        fprintf(stderr,"ERROR: unable to finalize mrapi %s\n",
                mrapi_display_status(status,status_buff,sizeof(status_buff))); 
        rc = MCAPI_FALSE;
      }
    }
    return rc;
  }

  /***************************************************************************
  NAME: transport_sm_get_shared_mem
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t  transport_sm_get_shared_mem(void** addr,
                                               uint32_t shmkey,
                                               uint32_t size) 
  {
    mcapi_status_t status;
    mrapi_boolean_t rc = MCAPI_FALSE;
    char status_buff[MRAPI_MAX_STATUS_SIZE];
    shm = mrapi_shmem_get (shmkey,&status);
    if (status == MRAPI_SUCCESS) {
      /* attach to the shared memory */
      *addr = mrapi_shmem_attach(shm,&status);
      if (status == MRAPI_SUCCESS) {
        rc = MCAPI_TRUE;
      }
    } 
    if (status != MRAPI_SUCCESS) {
      fprintf(stderr,"transport_sm_get_shared_mem failed.  mrapi_status=%s",
              mrapi_display_status(status,status_buff,sizeof(status_buff)));
    }
    return rc;
  }
  
  
  /***************************************************************************
  NAME: transport_sm_create_shared_mem
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t  transport_sm_create_shared_mem(void** addr,
                                                  uint32_t shmkey
                                                  ,uint32_t size) 
  {
    mcapi_status_t status;
    mrapi_boolean_t rc = MCAPI_FALSE;
    /* try to create the shared memory */
    shm = mrapi_shmem_create(shmkey,size,
                             NULL /* nodes list */,0 /* nodes list size */,
                             NULL /*attrs*/,0 /*attrs size*/,&status);
    if (status == MRAPI_SUCCESS) {
      /* attach to the shared memory */
      *addr = mrapi_shmem_attach(shm,&status);
      if (status == MRAPI_SUCCESS) {
        rc = MCAPI_TRUE;
      }
    } 
    return rc;
  }
  
  /***************************************************************************
  NAME: transport_sm_create_rwl
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t transport_sm_create_rwl(uint32_t key,
                                          uint32_t* handle,
                                          uint32_t num_readers) 
  {
    mcapi_status_t status;
    mcapi_boolean_t rc = MCAPI_FALSE;
    mrapi_rwl_hndl_t s;
    char status_buff [MRAPI_MAX_STATUS_SIZE];
    s = mrapi_rwl_create(key,NULL/*attrs*/,num_readers /*shared_lock_limit*/,&status);
    if (status == MRAPI_ERR_RWL_EXISTS) {
      s = mrapi_rwl_get(key,&status);
    }
    if (status == MRAPI_SUCCESS) {
      *handle = (uint32_t)s;
      rc = MCAPI_TRUE;
    } else {
      fprintf(stderr,"transport_sm_create_rwl failed status=%s\n",
              mrapi_display_status(status,status_buff,sizeof(status_buff)));
    }
    return rc;
  }
  
  /***************************************************************************
  NAME: transport_sm_lock_rwl
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t transport_sm_lock_rwl(uint32_t handle,
                                        mcapi_boolean_t write)
  {
    mcapi_status_t status;
    //char status_buff[MRAPI_MAX_STATUS_SIZE];
    mrapi_rwl_mode_t mode = (write) ? MRAPI_RWL_WRITER : MRAPI_RWL_READER;
    mrapi_rwl_lock (handle,mode,MRAPI_TIMEOUT_INFINITE /*timeout*/,&status);
    if (status == MRAPI_SUCCESS) {
      return MCAPI_TRUE;
    } else {
      //fprintf(stderr,"transport_sm_lock mrapi_status=%s handle=%x\n",
      // mrapi_display_status(status,status_buff,sizeof(status_buff)),handle);
      fflush(stdout);
      return MCAPI_FALSE;
    }
  }


  /***************************************************************************
  NAME: transport_sm_unlock_rwl
  DESCRIPTION:
  PARAMETERS: none
  RETURN VALUE: boolean: success or failure
  ***************************************************************************/
  mcapi_boolean_t transport_sm_unlock_rwl(uint32_t handle,
                                          mcapi_boolean_t write)
  {
    mcapi_status_t status;
    char status_buff[MRAPI_MAX_STATUS_SIZE];
    mrapi_rwl_mode_t mode = (write) ? MRAPI_RWL_WRITER : MRAPI_RWL_READER;
    mrapi_rwl_unlock(handle,mode,&status);
    if (status == MRAPI_SUCCESS) {
      return MCAPI_TRUE;
    } else {
      fprintf(stderr,"transport_sm_unlock mrapi_status=%s handle=%x\n",
              mrapi_display_status(status,status_buff,sizeof(status_buff)),
              (unsigned)handle);
      fflush(stdout);
      return MCAPI_FALSE;
    }
  }
  
  
#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
