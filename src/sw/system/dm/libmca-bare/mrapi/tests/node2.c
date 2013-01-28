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
This test makes sure that calling initialize for the same node/domain but from a 
different pid/tid will error out
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

#define LOCK_LIMIT 5

#define NUM_THREADS 1
#define WRONG(x) wrong(x,__LINE__);

void wrong(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s",line,mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

typedef struct {
  mca_node_t tid;
} thread_data;

int x;

void *run_thread(void* tdata) {
  mca_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;

  thread_data* t = (thread_data*)tdata;
  printf("run_thread...");
  fflush(stdout);
  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status == MRAPI_SUCCESS) {  WRONG(status) }

  return NULL;
}


int main () {
  int rc = 0;
  int i = 0;
  pthread_t threads[NUM_THREADS];
  thread_data tdata[NUM_THREADS];
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_set_debug_level(1);


  tdata[0].tid = 0;

  printf("main.....");
  mrapi_initialize(DOMAIN,NODE,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  fflush(stdout);

  rc += pthread_create(&threads[0], NULL, run_thread,(void*)&tdata[0]);

  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i],NULL);
  }
  
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  
  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  
  return rc;
}
