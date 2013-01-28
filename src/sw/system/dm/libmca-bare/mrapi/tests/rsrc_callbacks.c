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
 *   This tests makes sure we can manage the resource tree in a multi-node testcase.
 */

#include <mrapi.h>

char status_buff[MRAPI_MAX_STATUS_SIZE];

char status_buff[MRAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1 
#endif

#define NUM_THREADS 2


typedef struct {
  mca_node_t tid;
  mrapi_boolean_t test_event;
  mrapi_boolean_t buffer_nearly_full;
} thread_data;

/* global but indexed by tid so no race conditions should exist */
thread_data tdata[NUM_THREADS];


#define WRONG_STATUS(x) wrong_status(x,__LINE__);
void wrong_status(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s\n", line, mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}
#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

void crossbar_buffer_near_full(mrapi_event_t event) {
  mca_status_t status;
  mrapi_node_t node_id;
  
  node_id = mrapi_node_id_get(&status);
  printf("CALLBACK1: crossbar_buffer_near_full callback, node id %d\n", node_id); 
  if (node_id != 1) {
    mrapi_finalize(&status);
    printf("  FAIL: Incorrect node called this callback\n"); 
    exit(1);
    // or pthread_exit?
  }
  tdata[node_id-1].buffer_nearly_full = MRAPI_TRUE;
}

void test_callback(mrapi_event_t event) {
  mca_status_t status;
  mrapi_node_t node_id;
  
  node_id = mrapi_node_id_get(&status);
   printf("CALLBACK2: test_callback callback, node id %d\n", node_id); 
   if (node_id != 2) {
     mrapi_finalize(&status);
     printf("  FAIL: Incorrect node called this callback\n"); 
     exit(1);
     // or pthread_exit?
   }
   tdata[node_id-1].test_event = MRAPI_TRUE;
}


const char *print_tid() {
  static char buffer[100];
  char *p = buffer;
  pthread_t t = pthread_self();
#ifdef __linux
  /* We know that pthread_t is an unsigned long */
  sprintf(p, "%lu", t);
#else
  /* Just print out the contents of the pthread_t */ {
    char *const tend = (char *) ((&t)+1);
    char *tp = (char *) &t;
    while (tp < tend) {
      p += sprintf (p, "%02x", *tp);
      tp++;
      if (tp < tend)
        *p++ = ':';
    }
  }
#endif
  return buffer;
}

void *run_thread(void* threaddata) {
  mrapi_info_t          version;
  mrapi_parameters_t    parms = 0;
  int i;
  mca_status_t mrapi_status;

  thread_data* t = (thread_data*)threaddata;
  printf("run thread: t=%d TID: %s\n", t->tid, print_tid()); 

  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG_STATUS(mrapi_status) }

  /* Test the callback mechanism */
  mrapi_event_t crossbar_event = MRAPI_EVENT_CROSSBAR_BUFFER_OVER_80PERCENT;
  unsigned int crossbar_frequency = 3;
  if (t->tid % 2 == 0) {
    mrapi_resource_register_callback(crossbar_event, crossbar_frequency,
				     &crossbar_buffer_near_full, &mrapi_status);
    if (mrapi_status != MRAPI_SUCCESS) { WRONG_STATUS(mrapi_status) }
    if (tdata[t->tid-1].buffer_nearly_full != MRAPI_FALSE) { WRONG }
    printf("registering crossbar_buffer_near_full\n");

  } else {
    mrapi_resource_register_callback(crossbar_event, crossbar_frequency,
				     &test_callback, &mrapi_status);
    if (mrapi_status != MRAPI_SUCCESS) { WRONG_STATUS(mrapi_status) }
    if (tdata[t->tid-1].test_event != MRAPI_FALSE) { WRONG }
    printf("registering test_callback\n");
  }

  /* Sleep several times to allow the 3 alarms to trigger the callback */
  for (i = 0; i < 20; i++) {
    sleep(1);
  }

  printf("t=%d mrapi_finalize TID:%s\n",t->tid,print_tid());
  mrapi_finalize(&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG_STATUS(mrapi_status) }

  printf("t=%d DONE TID: %s\n",t->tid,print_tid());
  return NULL;
}

int main () {
  int rc = 0;
  int i = 0;
  pthread_t threads[NUM_THREADS];


  mrapi_set_debug_level(6);

  for (i = 0; i < NUM_THREADS; i++) {
    tdata[i].tid = i;
    tdata[i].test_event = MRAPI_FALSE;
    tdata[i].buffer_nearly_full = MRAPI_FALSE;
  }

  for (i = 0; i < NUM_THREADS; i++) {
    rc += pthread_create(&threads[i], NULL, run_thread, (void*)&tdata[i]);
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i],NULL);
  }

  /* Since the alarm in not tied to a specific node, any of the threads may */
  /* have had their callbacks invoked.  Check at least one of the callbacks */
  /* were invoked */
  int bnf = 0;
  int te = 0;
  for (i = 0; i < NUM_THREADS; i++) {
    if (tdata[i].buffer_nearly_full == MRAPI_TRUE) {bnf++;}
    if (tdata[i].test_event == MRAPI_TRUE) {te++;}
  }
  if ((bnf == 0) || (te == 0)) {
    rc = 1;
  }

  if (rc == 0) {
    printf("   Test PASSED\n");
  } else {
    printf("   Test FAILED bnf=%d te=%d\n",bnf,te);
  }

  return rc;
}
