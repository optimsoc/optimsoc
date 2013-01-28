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

                                                 
                                                 
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h> /* for yield */
#include <sys/shm.h>

#include <mrapi.h>
#include <mca_utils.h>
#include <mrapi_powerpc_lock.h>
#include <mrapi_impl_spec.h>  
  
  /***************************************************************************
  Function: sys_sem_create
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure

  ***************************************************************************/
  mrapi_boolean_t sys_sem_create(int key,int num_locks,int* id) {
    int i;

    /* critical section:
       We don't want anyone else creating/deleting while we are creating */
    acquire_lock(&mrapi_db->global_lock);
    
    /* first make sure it doesn't already exist */        
    for (i = 0; i < MRAPI_MAX_SEMS; i++) {
      if ((mrapi_db->sys_sems[i].key == key) && 
          (mrapi_db->sys_sems[i].valid)) {
        break;
      }
    }
    if (i == MRAPI_MAX_SEMS) {
      /* we didn't find it so create it */
      for (i = 0; i < MRAPI_MAX_SEMS; i++) {
        if (!mrapi_db->sys_sems[i].valid) {
          memset(&mrapi_db->sys_sems[i],0,sizeof(mrapi_sys_sem_t));
          mrapi_db->sys_sems[i].valid = MRAPI_TRUE;
          mrapi_db->sys_sems[i].key = key;
          mrapi_db->sys_sems[i].num_locks = num_locks;
        }
      }
    }
    release_lock(&mrapi_db->global_lock);
    if (i == MRAPI_MAX_SEMS) {
      *id = -1;
      return MRAPI_FALSE;
    } else {
      *id = i;
      return MRAPI_TRUE;
    }
  }
  
  /***************************************************************************
  Function: sys_sem_get

  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t sys_sem_get(int key, int num_locks, int* id) {
    
    int i;
    
    for (i = 0; i < MRAPI_MAX_SEMS; i++) {
      if (mrapi_db->sys_sems[i].key == key) {
        break;
      }
    }
    
    if (i == MRAPI_MAX_SEMS) {
      *id = -1;
      return MRAPI_FALSE;
    }
    *id = i;
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: sys_sem_trylock

  Description: This version of trylock will retry if another thread/process
    has the lock but will not retry if the lock has gone bad (been deleted).

  Parameters:

  Returns: boolean indicating success or failure

 ***************************************************************************/
  mrapi_boolean_t sys_sem_trylock (int id) {
    
    /* fix me: we currently only support mutexes but eventually
       need to support shared locks.  For now, just use index 0 */
    return try_lock(&mrapi_db->sys_sems[id].locks[0]);  
  }
  
  /***************************************************************************
  Function: sys_sem_lock
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t sys_sem_lock (int id,int member) {
   
     if (!mrapi_db) {
       return MRAPI_FALSE;
     } 
    /* fix me: we currently only support mutexes but eventually
       need to support shared locks.  For now, just use index 0 */
     acquire_lock(&mrapi_db->sys_sems[id].locks[member]);
     return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: sys_sem_unlock
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/  
  mrapi_boolean_t sys_sem_unlock (int id) {
     if (!mrapi_db) {
       return MRAPI_FALSE;
     }

    /* fix me: we currently only support mutexes but eventually
       need to support shared locks.  For now, just use index 0 */
     release_lock(&mrapi_db->sys_sems[id].locks[0]);
     return MRAPI_TRUE; 
 }
  
  /***************************************************************************
  Function: sys_sem_delete
  
  Description: detaches and frees the semaphore for the given id.
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t sys_sem_delete(int semid){
    int i = semid;

    if ((semid >= MRAPI_MAX_SEMS) || (semid < 0)) {
      return MRAPI_FALSE;
    }
    
    /* critical section:
       We don't want anyone else creating/deleting while we are deleting */
    acquire_lock(&mrapi_db->global_lock);
    mrapi_db->sys_sems[i].valid = MRAPI_FALSE;
    mrapi_db->sys_sems[i].key = 0;
    mrapi_db->sys_sems[i].num_locks = 0; 
    release_lock(&mrapi_db->global_lock);

    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: sys_shmem_detach
  
  Description: frees the shared memory for the given address.
  
  Parameters: shmem_address - shared memory address
  
  Returns: boolean indicating success or failure
  
  ***************************************************************************/
  mrapi_boolean_t sys_shmem_detach(void *shmem_address){ 
    
    /* The database should already be locked ! */
    
    mrapi_dprintf(1,"sys_shmem_detach addr=%p",shmem_address);
    /* detach the shared memory segment */
    int rc = shmdt(shmem_address);
    if (rc==-1) {
      mrapi_dprintf(1,"Warning: mrapi: sys_shmem_detach shmemdt() failed errno:%s",strerror(errno));
      return MRAPI_FALSE;
    }
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: sys_shmem_delete
  
  Description: frees the shared memory for the given id.
  
  Parameters: shmem_address - shared memory address
              id - shared memory id
              
  Returns:   
             
  ***************************************************************************/
  mrapi_boolean_t sys_shmem_delete(uint32_t id){ 
    
    int rc;
    struct shmid_ds shmid_struct;
    /* The database should already be locked ! */
    
    mrapi_dprintf(1,"sys_shmem_delete removing id=%d",id);
    
    /* delete the shared memory id */
    rc = shmctl(id, IPC_RMID, &shmid_struct);
    if (rc==-1)  {
      mrapi_dprintf(1,"Warning: mrapi: sys_shmem_delete shmemctl() failed errno: %s",strerror(errno));
      return MRAPI_FALSE;
    }
    return MRAPI_TRUE;
  }
  
  /***************************************************************************
  Function: sys_shmem_get
  
  Description: Returns shared memory segment id for a given key. 
  
  Parameters: shmemkey - shared memory key
              size - the desired size (in bytes)
              
  Returns: id: the shared memory id
              
  ***************************************************************************/
  uint32_t sys_shmem_get(uint32_t shmemkey,int size){
    
    /* the database should already be locked */
    uint32_t id = shmget(shmemkey, size, 0666 | IPC_CREAT); 
    
    mrapi_dprintf(1,"sys_shmem_get errno:%s",strerror(errno));
    
    return id;
  }
  
  /***************************************************************************
  Function: sys_shmem_create
  
  Description: Returns shared memory segment id for a given key. 
  
  Parameters: shmemkey - shared memory key
              size - the desired size (in bytes)
              
  Returns: id: the shared memory id
              
  ***************************************************************************/
  uint32_t sys_shmem_create(uint32_t shmemkey,int size){
    
    /* the database should already be locked */
    uint32_t id = shmget(shmemkey, size, 0666 | IPC_CREAT | IPC_EXCL); 
    
    mrapi_dprintf(1,"sys_shmem_create errno:%s",strerror(errno));
    
    return id;
  }
  
  /***************************************************************************
  Function: sys_shmem_attach
  
  Description: attaches the process to the shared memory for the given id.
    
  Parameters: id - shared memory id
  
  Returns: NULL->FAIL, otherwise address of shared memory segment
  
  ***************************************************************************/
  void* sys_shmem_attach(int id){ 
    
    /* the database should already be locked */
    struct shmid_ds dsbuf;
    void* addr = shmat(id, 0, 0);
    if ((long)addr == (-1)) {
      mrapi_dprintf(1,"Warning: mrapi sys_shmem_attach PPID %d: shmemat %u failed! errno %d '%s'", getpid(), (unsigned)id, errno, strerror(errno));
      return NULL;
    }
    
    if (shmctl(id, IPC_STAT, &dsbuf)) {
      mrapi_dprintf(1,"Warning: mrapi sys_shmem_attach PPID %d: shmemctl %u failed! errno %d '%s'", getpid(), (unsigned)id, errno, strerror(errno));
      shmdt(addr);
      return NULL;
    }
    
    /* if we are the first to attach, then initialize the segment to 0 */
    if (dsbuf.shm_nattch == 1) {
      memset(addr,0,dsbuf.shm_segsz);
    }
    
    mrapi_dprintf(1,"sys_shmem_attach: shmem:%u nattch:%d size:%d", id, (int)dsbuf.shm_nattch,(int)dsbuf.shm_segsz);
    return addr;
  }

#ifdef __cplusplus
  extern } 
#endif /* __cplusplus */
