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
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>   

#include <errno.h>
//#include <stdlib.h>
#include <stdio.h>    
#include <string.h>

void* addr;
int shmid;
int semid; 

#define TRUE 0
#define FALSE 1

int create_sem() {
  int rc = TRUE; 
  key_t key = 0xbeefdead;
  int num_locks = 1;
  struct sembuf sbuf;

  semid = semget(key, num_locks, IPC_CREAT|IPC_EXCL|0666);
  if (semid == -1) {
    printf("create_sem failed errno=%s\n",strerror(errno));
    rc = FALSE;
  } else {
    /* initialize the semaphore */
    sbuf.sem_num = 0;
    sbuf.sem_op = 2;
    sbuf.sem_flg = 0;
    if (semop(semid,&sbuf,1) == -1) {
      printf("create_sem failed during init errno=%s\n",strerror(errno));
      rc = FALSE;
    }
  }



  return rc;
}

int unlock_sem() {
  int rc = TRUE;
  int member = 0;
  struct sembuf sem_unlock={ member, 1, 0};
  
  /* Attempt to unlock the semaphore set */
  if((semop(semid, &sem_unlock, 1)) == -1) {
    printf("unlock_sem failed errno=%s\n",strerror(errno));
    rc = FALSE;
  }
  return rc;
}


int lock_sem() {
  int rc = TRUE;
  int member = 0;
  struct sembuf sem_lock={ member, -1, IPC_NOWAIT};
  if((semop(semid, &sem_lock, 1)) == -1) {
    printf("lock_sem failed errno=%s\n",strerror(errno));
    rc = FALSE;
  }
  return rc;
}

int free_sem() {
  int rc = TRUE;
  int r;
  r = semctl( semid, 0, IPC_RMID,0 );
  if (r==-1) {
    printf("free_sem failed errno=%s\n",strerror(errno));
    rc =FALSE;
  }
  return rc;
}

int create_and_attach_shm() {
  int rc = TRUE;
  key_t shmemkey = 0xbabecafe;
  size_t size = 1024;
  shmid = shmget(shmemkey, size, 0666 | IPC_CREAT | IPC_EXCL);
  if (shmid == -1) {
    printf("shm create (shget) failed errno=%s\n",strerror(errno));
    rc = FALSE;;
  } else {
    struct shmid_ds dsbuf;
    addr = shmat(shmid, 0, 0);
    if ((long)addr == (-1)) {
      printf("shm create (shmat) failed errno=%s\n",strerror(errno));
      rc = FALSE;
    } else {
      if (shmctl(shmid, IPC_STAT, &dsbuf)) {
        printf("shm create (shctl) failed errno=%s\n",strerror(errno));
        rc = FALSE;
        shmdt(addr);
      } else {
        /* if we are the first to attach, then initialize the segment to 0 */
        if (dsbuf.shm_nattch == 1) {
          memset(addr,0,dsbuf.shm_segsz);
        }        
      }
    }
  }
  return rc;
}

int free_shm() {
  int rc;
  struct shmid_ds shmid_struct;
  
  // detach
  rc = shmdt(addr); 
  if (rc==-1)  {
    printf("free_shm (shmdt) failed errno=%s\n",strerror(errno));
    rc = FALSE;
  }
  
  /* delete the shared memory id */
  rc = shmctl(shmid, IPC_RMID, &shmid_struct);
  if (rc==-1)  {
    printf("free_shm (shmctl) failed errno=%s\n",strerror(errno));
    rc = FALSE;
  }
  return rc;
}

 
int main() {
  int* a;
  int rc = TRUE;
  printf("creating sem...\n");
  rc |= create_sem();
  printf("locking sem...\n");
  rc |= lock_sem();
  printf("creating and attaching to shmem...\n");
  rc |= create_and_attach_shm();
  a = (int*)addr;
  // make sure we can read/write the shared memory
  *a = 1;
  if (*a != 1) {
    printf("writing shared memory failed\n");
    rc |= 1;
  } else {
    printf("Wrote %i to shared memory\n",*a);
  }
  printf("freeing resources...\n");
  rc |= free_shm();
  rc |= unlock_sem();
  rc |= free_sem();
  return rc;
}
