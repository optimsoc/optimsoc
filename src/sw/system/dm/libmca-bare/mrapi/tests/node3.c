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
/*
This test makes sure that we correctly check the node limit (MRAPI_MAX_NODES) 
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

#define NUM_THREADS (MCA_MAX_NODES +1)

#define WRONG(x) wrong(x,__LINE__)

void wrong(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s",line,mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

typedef struct {
  mca_node_t tid;
  mrapi_status_t status;
  mca_domain_t domain;
} thread_data;

volatile int done = 0;
volatile int expected_fail_done = 0;

void *run_thread(void* tdata) {
  mrapi_status_t mrapi_status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_boolean_t creator = MRAPI_FALSE;
  thread_data* t = (thread_data*)tdata;
  mrapi_sem_hndl_t sem;
  unsigned semkey = 0xdeadbeef;

  printf("run_thread (t=%d)...\n",t->tid);
  fflush(stdout);
  
  /* initialize */
  printf("t=%d initialize\n",t->tid);
  mrapi_initialize(DOMAIN+t->domain,NODE+t->tid,parms,&version,&t->status);
  printf("t=%d create or get sem\n",t->tid);
  if (t->status  == MRAPI_SUCCESS) {
    printf("t=%d sem_create\n",t->tid);    
    /* create or get the semaphore */
    sem=mrapi_sem_create(semkey,NULL,LOCK_LIMIT,&mrapi_status);
    if (mrapi_status  != MRAPI_SUCCESS) { 
      sem = mrapi_sem_get(semkey,&mrapi_status);
      if (mrapi_status  != MRAPI_SUCCESS) { WRONG(mrapi_status);}
    } else {
      creator = MRAPI_TRUE;
    }
    printf("t=%d sem_lock\n",t->tid); 
    mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
    if (mrapi_status != MRAPI_SUCCESS) { WRONG(mrapi_status);}
   
    printf("done=%d\n",done); 
    done++;
    
    mrapi_sem_unlock(sem,&mrapi_status);
    if (mrapi_status != MRAPI_SUCCESS) { WRONG(mrapi_status);}
    
    while ((expected_fail_done == 0) || (done < (NUM_THREADS-1))) {} 
    if (creator) {
      mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
      if (mrapi_status != MRAPI_SUCCESS) { WRONG(mrapi_status);}
      mrapi_sem_delete (sem,&mrapi_status);
      if (mrapi_status != MRAPI_SUCCESS) { WRONG(mrapi_status);}
    }
    mrapi_finalize(&mrapi_status);
    if (mrapi_status != MRAPI_SUCCESS) { WRONG(mrapi_status); } 
  } else {
    expected_fail_done++;
    printf("init failed (we expected this for one thread) t=%d status=%s\n",t->tid,mrapi_display_status(t->status,status_buff,sizeof(status_buff)));
  }
  
  return NULL;
}


int main () {
  int rc = 1;
  int i = 0;
  pthread_t threads[NUM_THREADS];
  thread_data tdata[NUM_THREADS];
  int init_passed = 0;
  int init_failed = 0;
 
  mrapi_set_debug_level(6);

  printf("MCA_MAX_NODES=%d, MCA_MAX_DOMAINS=%d, node3 NUM_THREADS=%d\n",MCA_MAX_NODES,MCA_MAX_DOMAINS,NUM_THREADS);
  for (i = 0; i < NUM_THREADS; i++) {
    tdata[i].tid = i;
    tdata[i].domain = 0;
    tdata[i].status = 0;
  }

  printf("testing MCA_MAX_NODES...\n");
  for (i = 0; i < NUM_THREADS; i++)
    rc += pthread_create(&threads[i], NULL, run_thread,(void*)&tdata[i]);
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i],NULL);
  }
  
  /* check results */
 for (i = 0; i < NUM_THREADS; i++) {
   if (tdata[i].status == MRAPI_ERR_NODE_INITFAILED) {
     init_failed++;
   }
   if (tdata[i].status == MRAPI_SUCCESS) {
     init_passed++;
   }
 }

 if ((init_failed == 1) && (init_passed == (NUM_THREADS-1))) {
   rc = 0;
 }

  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  
  return rc;
}
