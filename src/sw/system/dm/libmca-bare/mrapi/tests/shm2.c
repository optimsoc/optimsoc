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
#include <mrapi.h>

char status_buff[MRAPI_MAX_STATUS_SIZE];

char status_buff[MRAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


#define NUM_THREADS 2
#define WRONG(x) wrong(__LINE__,x);
#define SEM_KEY 0xdeadbeef
#define SHMEM_KEY 0xdeadbabe
#define LOCK_LIMIT 1

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define RESULT 1024 

unsigned* addr;
unsigned  data;

typedef struct {
  mrapi_sem_hndl_t sem; /* mrapi sem handle */
  mrapi_shmem_hndl_t shmem; /* mrapi shmem handle */
  mca_node_t tid;
} thread_data;

void wrong(unsigned line,thread_data t)
{
  mca_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mrapi_sem_delete (t.sem,&status);
  mrapi_shmem_detach(t.shmem,&status);
  mrapi_shmem_delete(t.shmem,&status);
  mrapi_finalize(&status);
  exit(1);
}


void *run_thread(void* tdata) {
  mca_status_t status;
  int i;
  thread_data* t = (thread_data*)tdata;
  mrapi_info_t version;
 mrapi_parameters_t parms = 0;

  printf("run_thread tid=%d",t->tid);
  /* initialize mrapi */
  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); fflush(stdout); WRONG(*t) }
  
  /* create or get the semaphore */
  t->sem = mrapi_sem_create(SEM_KEY,NULL /*attributes*/,LOCK_LIMIT,&status);
  if (status == MRAPI_ERR_SEM_EXISTS) {
    printf("sem exists, try getting it from the other node");
    t->sem = mrapi_sem_get(SEM_KEY,&status);
  }
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  /* create or get the shared memory and attach to it */
  t->shmem = mrapi_shmem_create(SHMEM_KEY,sizeof(int) /*size*/,NULL /* nodes list */,0 /* nodes list size */,NULL /*attrs*/,0 /*attrs size*/,&status);
 
  if (status == MRAPI_ERR_SHM_EXISTS) {
    printf("shmem exists, try getting it from the other node");
    t->shmem = mrapi_shmem_get(SHMEM_KEY,&status);
  }
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  addr = mrapi_shmem_attach(t->shmem,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
  
  /* read, increment and write shared memory */
  for (i = 0; i < RESULT/2; i++) {
    mrapi_sem_lock(t->sem,MRAPI_TIMEOUT_INFINITE /*timeout*/,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
    *(addr) = *(addr) + 1;
    //printf("t->tid:%d data=%d\n",t->tid,(unsigned)(*(addr)));
    data = *(addr);
    mrapi_sem_unlock(t->sem,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
  }

 // make sure one thread doesn't run to completion before the other thread even starts
  sched_yield();
  sleep(2);

  /* detach from the shared memory */
  mrapi_shmem_detach(t->shmem,&status);
  //if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
 
  /* finalize mrapi (it will free any remaining sems & shmems for us) */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  return NULL;
}


int main () {
 
  int rc = 0;
  int i = 0;
  thread_data tdata[NUM_THREADS];
  pthread_t threads[NUM_THREADS];
  

  mrapi_set_debug_level(1);


  tdata[0].tid = 0;
  tdata[1].tid = 1;

  
  rc += pthread_create(&threads[0], NULL, run_thread,(void*)&tdata[0]);
  rc += pthread_create(&threads[1], NULL, run_thread,(void*)&tdata[1]);

  for (i = 0; i < 2; i++) {
   pthread_join(threads[i],NULL);
  }  
 
  if (data != RESULT) { fprintf(stderr,"FAILED: expected addr=%u, found addr=%u\n",RESULT,data); rc=1; }

  if (rc == 0) {
    printf("   Test PASSED\n");
  }
 
 
  return rc;
}
