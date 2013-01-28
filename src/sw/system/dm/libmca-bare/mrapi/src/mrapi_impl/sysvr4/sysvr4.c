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

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h> /* for signals */
                                         
#include <mrapi.h>
#include <mca_utils.h>
#include "mrapi_sys.h"
                                                  
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h> /* for yield */
                                                  

/* Good System V IPC reference: 
  http://beej.us/guide/bgipc/output/html/multipage/semaphores.html#semsamp
*/

/***************************************************************************
  Function: sys_sem_create
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
 ***************************************************************************/
mrapi_boolean_t sys_sem_create(int key,int num_locks,int* id) {
  union semun {
    int val;
    struct semid_ds *buf;
    short *array;
  } arg;

  int max_semaphores_per_array = 250;
  /* Wish I could just get this from an include somewhere...*/
  /* To find SEMMSL kernel parameter:
    $ ipcs -ls
    
    ------ Semaphore Limits --------
    max number of arrays = 128
    max semaphores per array = 250
    max semaphores system wide = 32000
    max ops per semop call = 32
    semaphore max value = 32767
  */
  if (num_locks > max_semaphores_per_array) {
    printf("sys_sem_create failed: num_locks requested is greater then the OS supports (SEMMSL).\n");
    return MRAPI_FALSE;
  }

  mrapi_dprintf(1,"sys_sem_create (create)");
  /* 1. create the semaphore */
  *id = semget(key, num_locks, IPC_CREAT|IPC_EXCL|0666);
  if (*id == -1) {
    mrapi_dprintf(1,"sys_sem_create failed: errno=%s",strerror(errno));
    return MRAPI_FALSE;
  }
  mrapi_dprintf(1,"sys_sem_create (initialize)");
  /* 2. initialize all members (Note: create and initialize are NOT atomic!  
     This is why semget must poll to make sure the sem is done with
     initialization */

  struct sembuf sb;
  sb.sem_op = 1; 
  sb.sem_flg = 0;
  arg.val = 1;
  
  for(sb.sem_num = 0; sb.sem_num < num_locks; sb.sem_num++) { 
    /* do a semop() to "free" the semaphores. */
    /* this sets the sem_otime field, as needed below. */
    if (semop(*id, &sb, 1) == -1) {
      int e = errno;
      semctl(*id, 0, IPC_RMID); /* clean up */
      errno = e;
      return MRAPI_FALSE; /* error, check errno */
    }
  }
  
  return MRAPI_TRUE;
}

/***************************************************************************
  Function: sys_sem_get

  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
***************************************************************************/
mrapi_boolean_t sys_sem_get(int key, int num_locks, int* id) {
  
  union semun {
    int val;
    struct semid_ds *buf;
    short *array;
  } arg;
  
  int i;
  int ready = 0;
  int MAX_RETRIES = 0xffff;
  struct semid_ds buf;

  
  mrapi_dprintf(1,"sys_sem_get");
  
  *id = semget(key, num_locks, 0); /* get the id */
  if (*id == -1) {
    mrapi_dprintf(1,"sys_sem_get failed: errno=%s",strerror(errno));
    return MRAPI_FALSE;
  }
  /* 
  At that point, process 2 will have to wait until the semaphore is initialized 
  by process 1. How can it tell? Turns out, it can repeatedly call semctl() with 
  the IPC_STAT flag, and look at the sem_otime member of the returned struct 
  semid_ds structure. If that's non-zero, it means process 1 has performed an 
  operation on the semaphore with semop(), presumably to initialize it.
  */
  
  /* wait for other process to initialize the semaphore: */
  arg.buf = &buf;
  for(i = 0; i < MAX_RETRIES && !ready; i++) {
    semctl(*id, num_locks-1, IPC_STAT, arg);
    if (arg.buf->sem_otime != 0) {
      ready = 1;
    } else {
      sleep(1); 
    }
  }
  if (!ready) {
    errno = ETIME;
    return MRAPI_FALSE;
  }
  
  return MRAPI_TRUE;
}

/***************************************************************************
  Function: sys_sem_trylock

  Description: This version of trylock will retry if another thread/process
    has the lock but will not retry if the lock has gone bad (been deleted).

  Parameters:

  Returns: boolean indicating success or failure

 ***************************************************************************/
  mrapi_boolean_t sys_sem_trylock (int id,int member) {
    struct sembuf sem_lock={ member, -1, IPC_NOWAIT};
    int rc = 0;
    mrapi_dprintf(1,"sys_sem_trylock");
    // retry only if we get EINTR
    while (1) {
      rc = semop(id, &sem_lock, 1); 
      if ((rc == -1) && (errno != EINTR)) { 
        mrapi_dprintf(3,"sys_sem_trylock failed: errno=%s",strerror(errno)); 
        return MRAPI_FALSE;
      }
      if (rc >= 0) { 
        return MRAPI_TRUE;
      }
      mrapi_dprintf(6,"sys_sem_trylock attempt failed: errno=%s",strerror(errno));
    } 
    
    return MRAPI_FALSE;
  }

/***************************************************************************
  Function: sys_sem_lock
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
 ***************************************************************************/
  mrapi_boolean_t sys_sem_lock (int id,int member) {
  
  while(1) {
    // repeatedly call trylock until we get the lock or fail due to an 
    // error other than EAGAIN (someone else has the lock).
    if (sys_sem_trylock(id,member)) {
      return MRAPI_TRUE;
    } else if (errno != EAGAIN) {
      mrapi_dprintf(2,"sys_sem_lock attempt failed: errno=%s",strerror(errno));
    }
  }
  return MRAPI_FALSE;  
} 

/***************************************************************************
  Function: sys_sem_unlock
  
  Description: 
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
 ***************************************************************************/  
  mrapi_boolean_t sys_sem_unlock (int id,int member) {
  
  struct sembuf sem_unlock={ member, 1, 0};
  mrapi_dprintf(4,"sys_sem_unlock");
  if((semop(id, &sem_unlock, 1)) == -1) {
    mrapi_dprintf(1,"sys_sem_unlock failed: errno=%s",strerror(errno));
    return MRAPI_FALSE;
  }
  return MRAPI_TRUE;
}
                                             
/***************************************************************************
  Function: sys_sem_delete
  
  Description: detaches and frees the semaphore for the given id.
  
  Parameters: 
  
  Returns: boolean indicating success or failure
  
 ***************************************************************************/
mrapi_boolean_t sys_sem_delete(int semid){ 
  mrapi_boolean_t rc = MRAPI_TRUE;
  int r;
  r = semctl( semid, 0, IPC_RMID,0 );
  mrapi_dprintf(1,"sys_sem_delete removing semaphore:%d",semid);
  if (r==-1) {
    printf("ERROR: unable to remove semaphore: semctl() remove id failed errno: %s\n",strerror(errno));
    rc =MRAPI_FALSE;
  }
  return rc;
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
      printf("ERROR: mrapi: sys_shmem_detach shmemdt() failed errno:%s\n",strerror(errno));
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
      printf("ERROR: mrapi: sys_shmem_delete shmemctl() failed errno: %s\n",strerror(errno));
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
   
#if 0 
  /* if we are the first to attach, then initialize the segment to 0 */
    if (dsbuf.shm_nattch == 1) {
      memset(addr,0,dsbuf.shm_segsz);
    }
#endif    
    mrapi_dprintf(1,"sys_shmem_attach: shmem:%u nattch:%d size:%d", id, (int)dsbuf.shm_nattch,(int)dsbuf.shm_segsz);
    return addr;
  }

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
