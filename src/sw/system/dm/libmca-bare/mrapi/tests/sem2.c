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

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define LOCK_LIMIT 1

#define NUM_THREADS 2
#define WRONG wrong(__LINE__);

#define N 12

void wrong(unsigned line)
{
  mrapi_status_t status;
  fprintf(stderr,"WRONG: line=%u ", line);
  mrapi_finalize(&status);
  fflush(stdout);
  exit(1);
}

typedef struct {
  mca_node_t tid;
} thread_data;

volatile int x;

void *run_thread(void* tdata) {
  mca_status_t status;
  int semkey = 1;
  int i = 0;
  mrapi_info_t version;
  mrapi_sem_hndl_t sem;
  mrapi_parameters_t parms = 0;
  mrapi_boolean_t creator = MRAPI_TRUE;

  thread_data* t = (thread_data*)tdata;

  fprintf(stderr,"run_thread start tid=%d\n",t->tid);

  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { WRONG }
  printf("initialize completed!!!!!!!!!!!!!!!!!!!!!!\n");
  sem=mrapi_sem_create(semkey,NULL /*atttrs */,LOCK_LIMIT,&status);
  if (status == MRAPI_ERR_SEM_EXISTS)
  {
    sem=mrapi_sem_get(semkey,&status);
    creator = MRAPI_FALSE;
  }
  if (status != MRAPI_SUCCESS) { WRONG }

  for (i = 0; i < N/NUM_THREADS; i++) {
    mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG }
    x++;
    printf("x=%d\n",x);
    mrapi_sem_unlock(sem,&status);
    if (status != MRAPI_SUCCESS) { WRONG }
  }

 
  /* synchronize the two threads before calling freeing the sem */
  while (x != N)
    continue;
  
  if (creator) {
    mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG }
    
    mrapi_sem_delete (sem,&status);
    if (status != MRAPI_SUCCESS) { WRONG }
  } else {    
    sleep(2);
  }

  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { WRONG }
  fprintf(stderr,"run_thread done tid=%d\n",t->tid);
  return NULL;
}


int main () {
  int rc = 0;
  int i = 0;
  pthread_t threads[NUM_THREADS];
  thread_data tdata[NUM_THREADS];


  mrapi_set_debug_level(1);


  tdata[0].tid = 0;
  tdata[1].tid = 1;

  rc += pthread_create(&threads[0], NULL, run_thread,(void*)&tdata[0]);
  rc += pthread_create(&threads[1], NULL, run_thread,(void*)&tdata[1]);

  
  for (i = 0; i < 2; i++) {
    pthread_join(threads[i],NULL);
  }
  
  if (x != N) { WRONG; }
  
  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  
  return rc;
}
