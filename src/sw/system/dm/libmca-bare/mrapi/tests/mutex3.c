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
/* Basic test for recursive locking of mutexes */

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


#define LOCK_LIMIT 5

#define NUM_THREADS 2
#define WRONG(x) wrong(x,__LINE__);

#define NUM_ITERATIONS 10


void wrong(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s",line,mrapi_display_status(status,status_buff,sizeof(status_buff)));
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
  int mutexkey = 1;
  int i = 0;
  mrapi_info_t version;
  mrapi_mutex_hndl_t mutex;
  mrapi_parameters_t parms = 0;
  mrapi_key_t lock_key[NUM_ITERATIONS];
  mrapi_boolean_t creator = MRAPI_TRUE;
  mrapi_mutex_attributes_t attrs;
  mrapi_boolean_t recursive_attr = MRAPI_TRUE;
  mrapi_boolean_t recursive_attr2 = MRAPI_FALSE;

  thread_data* t = (thread_data*)tdata;

  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) {  WRONG(status) }

  mrapi_mutex_init_attributes(&attrs,&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }

 /* test invalid attributes parameter */
  mrapi_mutex_set_attribute(NULL,MRAPI_MUTEX_RECURSIVE,&recursive_attr,sizeof(recursive_attr),&status);
  if (status != MRAPI_ERR_PARAMETER) { WRONG(status) }

/* test invalid attribute number */
  mrapi_mutex_set_attribute(&attrs,42,&recursive_attr,sizeof(recursive_attr),&status);
  if (status != MRAPI_ERR_ATTR_NUM) { WRONG(status) }

 /* test invalid attribute size */
  mrapi_mutex_set_attribute(&attrs,MRAPI_MUTEX_RECURSIVE,&recursive_attr,sizeof(void*),&status);
  if (status != MRAPI_ERR_ATTR_SIZE) { WRONG(status) }

  mrapi_mutex_set_attribute(&attrs,MRAPI_MUTEX_RECURSIVE,&recursive_attr,sizeof(recursive_attr),&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }

  mutex=mrapi_mutex_create(mutexkey,&attrs,&status);
  if (status == MRAPI_ERR_MUTEX_EXISTS)
  {
    mutex=mrapi_mutex_get(mutexkey,&status);
    creator = MRAPI_FALSE;
  }
  if (status != MRAPI_SUCCESS) { WRONG(status) }

  /* test get attribute */
  mrapi_mutex_get_attribute(mutex,MRAPI_MUTEX_RECURSIVE,&recursive_attr2,sizeof(recursive_attr2),&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  if (recursive_attr2 != MRAPI_TRUE) { printf("recursive_attr2=%d\n",recursive_attr2); WRONG(status) }

  /* test recursive locking */
  for (i=0; i< NUM_ITERATIONS; i++) {
    mrapi_mutex_lock(mutex,&lock_key[i],MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
    x++;
  }

  /* test sending incorrect key */
  mrapi_mutex_unlock(mutex,&lock_key[0],&status);
  if (status != MRAPI_ERR_MUTEX_LOCKORDER) { WRONG(status) }
 
  /* test recursive unlocking */ 
  for (i=NUM_ITERATIONS-1; i>=0; i--) {
    mrapi_mutex_unlock(mutex,&lock_key[i],&status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
  }
  
  mrapi_mutex_lock(mutex,&lock_key[0],MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  x++;
  mrapi_mutex_unlock(mutex,&lock_key[0],&status);
  

  /* synchronize the threads */
  while (x != NUM_ITERATIONS*NUM_THREADS+NUM_THREADS)
    continue;

  printf("tid=%d, x=%d\n",t->tid,x);

  if (creator) {
    printf("CREATOR DELETING MUTEX\n");
    mrapi_mutex_lock(mutex,&lock_key[0],MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
    
    mrapi_mutex_delete (mutex,&status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
  } else {    
    sleep(2);
  }

  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }

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
  
  if (x != NUM_ITERATIONS*NUM_THREADS+NUM_THREADS) { WRONG(0) }
  
  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  
  return rc;
}
