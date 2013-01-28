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
#include "barrier1.h"

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#define NUM_THREADS 10

#ifndef WRONG
#define WRONG(x) wrong(x,__LINE__);
void wrong(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s",line,mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}
#endif

typedef struct {
  mca_node_t tid;
} thread_data;


/* we use two barriers to avoid the race-condition where the first thread
   through, resets the barrier before the others notice that all threads have reached it */
volatile int barrier1;
volatile int barrier2;




/*****************************************************************************
 * daxpy -- compute alpha * x + y where alpha is a scalar and x and y are n-vectors
 *****************************************************************************/
void daxpy(double y[], double a, double x[], int n)
{
  int i;

  for (i = 0; i < n; i++) {
    y[i] = a * x[i] + y[i];
  }
}


void *run_thread(void* tdata) {
  mca_status_t status;
  int mutexkey = 1;
  mrapi_info_t version;
  mrapi_mutex_hndl_t mutex;
  mrapi_parameters_t parms = 0;

  mrapi_boolean_t creator = MRAPI_TRUE;
  
  thread_data* t = (thread_data*)tdata;

  int       daxpy_size = 1000;
  double*   y;
  double*   x;
  x = (double *) malloc(daxpy_size * sizeof(double));
  y = (double *) malloc(daxpy_size * sizeof(double));
  
  printf("tid=%d\n",t->tid);
  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) {  WRONG(status) }
 
  mutex=mrapi_mutex_create(mutexkey,NULL /*atttrs */,&status);
  if (status == MRAPI_ERR_MUTEX_EXISTS)
  {
    mutex=mrapi_mutex_get(mutexkey,&status);
    creator = MRAPI_FALSE;
  }
  if (status != MRAPI_SUCCESS) { printf("FAIL tid=%d\n",t->tid); WRONG(status) }

  barrier(mutex,&barrier1,NUM_THREADS);

  // do some work
  daxpy(y, 3.1415, x, daxpy_size);

  barrier(mutex,&barrier2,NUM_THREADS);

  /* finalize will free up the mutex */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }

  return NULL;
}


int main () {
  int rc = 0;
  int i = 0;
  pthread_t threads[NUM_THREADS];
  thread_data tdata[NUM_THREADS];


  mrapi_set_debug_level(3);

  for (i = 0; i < NUM_THREADS; i++) { 
    tdata[i].tid = i;
  }

  for (i = 0; i < NUM_THREADS; i++) { 
    rc += pthread_create(&threads[i], NULL, run_thread,(void*)&tdata[i]);
  }
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i],NULL);
  }
  
  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  
  return rc;
}
