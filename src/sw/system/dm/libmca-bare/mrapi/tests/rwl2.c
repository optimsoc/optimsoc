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


/* Big rwl test with many threads and many locks */

#include <mrapi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#define NUM_LOCKS MRAPI_MAX_SEMS
   //#define NUM_THREADS MCA_MAX_NODES  takes too long
#define NUM_THREADS 8

#define RWL_KEY 0xdead0000
#define SHMEM_KEY 0xbabe0000
#define MY_TIMEOUT 0xffffffff

#define WRONG wrong(__LINE__,status);
void wrong(unsigned line,mca_status_t status) {
  char status_buff[MRAPI_MAX_STATUS_SIZE];
  fprintf(stderr,"WRONG: line=%u status=%s\n", line,
          mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  mrapi_finalize(&status);
  printf("FAILED\n");
  exit(1);
}


/* each thread will have an array of locks that it either creates or gets (depending on timing)
   and a corresponding shared memory segment for each lock */
typedef struct {
  mrapi_rwl_hndl_t rwl[NUM_LOCKS];
  mrapi_shmem_hndl_t shmem[NUM_LOCKS]; /* mrapi shmem handle */
  int* addr[NUM_LOCKS]; /* the addresses of each shmem segment */
  int tid;
  int rc;
} thread_data;


void *run_thread(void* tdata) {
  mca_status_t status;
  int i,l;
  int timeout;
  mrapi_boolean_t done;
  thread_data* t = (thread_data*)tdata;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_boolean_t attr = MRAPI_TRUE;
  mrapi_rwl_attributes_t attrs;
  int read_data;
  printf("tid:%d run_thread\n",t->tid);
  
  /* initialize this node */
  mrapi_initialize(DOMAIN,t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { WRONG }
      
  
  /* enable extended error checking */
  mrapi_rwl_init_attributes(&attrs,&status);
  if (status  != MRAPI_SUCCESS) { WRONG}
  mrapi_rwl_set_attribute(&attrs,MRAPI_ERROR_EXT,&attr,sizeof(attr),&status);
  if (status  != MRAPI_SUCCESS) { WRONG}

  /* create or get all the reader/writer locks */
  for (i = 0; i < NUM_LOCKS; i++) {
    t->rwl[i] = mrapi_rwl_create(RWL_KEY+i,NULL /*attributes*/,NUM_THREADS,&status);
    if (status == MRAPI_ERR_RWL_EXISTS) {
      //printf("tid:%d rwl already exists, try getting it...\n",t->tid);
      t->rwl[i] = mrapi_rwl_get(RWL_KEY,&status);
    }
    if (status != MRAPI_SUCCESS) { WRONG }
  }

  /* create or get the shared memory and attach to it */
  for (i = 0; i < NUM_LOCKS; i++) {
    t->shmem[i] = mrapi_shmem_create(SHMEM_KEY+i,sizeof(int) /*size*/,NULL /* nodes list */,0 /* nodes list size */,NULL /*attrs*/,0 /*attrs size*/,&status);
    if (status == MRAPI_ERR_SHM_EXISTS) {
      //printf("tid:%d shmem exists, try getting it...\n",t->tid);
      t->shmem[i] = mrapi_shmem_get(SHMEM_KEY+i,&status);
    }
    if (status != MRAPI_SUCCESS) { WRONG }
    t->addr[i] = mrapi_shmem_attach(t->shmem[i],&status);
    if (status != MRAPI_SUCCESS) { WRONG }
  }
  
  /* read and write the shared memory */
  for (i = 0; i < NUM_LOCKS; i++) {
    /* write the shared memory */
    mrapi_rwl_lock(t->rwl[i],MRAPI_RWL_WRITER,MY_TIMEOUT /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG}
    *(t->addr[i]) = *(t->addr[i]) + 1;
    //printf("t->tid:%d wrote:%d to entry:%d (%p) \n",t->tid,(*(t->addr[i])),i,t->addr);    
    mrapi_rwl_unlock(t->rwl[i],MRAPI_RWL_WRITER,&status);
    if (status != MRAPI_SUCCESS) { WRONG}
    sched_yield();
    /* read the shared memory */
    mrapi_rwl_lock(t->rwl[i],MRAPI_RWL_READER,MY_TIMEOUT/*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG}
    read_data = *(t->addr[i]);
    //printf("t->tid:%d read:%d from entry:%d (%p) \n",t->tid,read_data,i,t->addr); 
    mrapi_rwl_unlock(t->rwl[i],MRAPI_RWL_READER,&status);
    if (status != MRAPI_SUCCESS) { WRONG}
  }

  /* wait for all threads to finish incrementing and check results */
  timeout = 0;
  done = MRAPI_FALSE;
  while ((!done) && (timeout < MY_TIMEOUT)) {
    timeout++;
    done = MRAPI_TRUE;
    for (l = 0; l < NUM_LOCKS; l++) {
      /* walk my shmem array and see if each thread has incremented each of the shmem segments */
      if ( *(t->addr[l]) != NUM_THREADS) {
        done = MRAPI_FALSE;
      }
    }
  }

  if (!done) {
    printf("UGH: tid:%d hit timeout\n",t->tid);
    t->rc = 1;
    for (l = 0; l < NUM_LOCKS; l++) {
      if ( *(t->addr[l]) != NUM_THREADS) {
        printf("t->addr[%d] (%p) = %d, expected %d\n",l,t->addr[l],*t->addr[l],NUM_THREADS);
      }
    }
  } 

  /* finalize */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { WRONG}
  
  return NULL;
}

int main () {
  thread_data tdata_array[NUM_THREADS];
  int i;
  pthread_t threads[NUM_THREADS];
  int rc = 0;

  mrapi_set_debug_level(1);

  assert (NUM_THREADS < MCA_MAX_NODES);

  /* init the thread data */
  for (i=0; i < NUM_THREADS; i++) {
    tdata_array[i].tid = i;
    tdata_array[i].rc = 0;
  }
  
  /* run the threads */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, run_thread,(void*)&tdata_array[i]);
  }
  
  /* wait for them all to complete */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i],NULL);
  }
  
  /* check results */
  for (i=0; i < NUM_THREADS; i++) {
    rc += tdata_array[i].rc;
  }

  if (rc == 0) {
    printf("PASSED\n");
  } else {
    printf("FAILED\n");
  }
  return rc;
}
