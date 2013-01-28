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
  Test: msg3
  Description: This is a pthreads test that sends 256 messages using a sender thread 
  and a receiver thread.  The sender can (and often does) overflow the receive queue.  
  This is of course, dependent on the size of the receive queue and the scheduling 
  algorithm.  This test is run twice, once with the receiver first and once with the 
  sender first.  The sender will re-try TIMEOUT times if a send fails (usually due to
  the queue being full - ENO_BUFFER).  The receiver will re-try TIMEOUT times if a 
  recv fails (usually due to empty queue).

  Tests blocking send/recv.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> /* for exit */

#define NUM_THREADS 2

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define NUM_MSGS 256

  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;

typedef struct {
  uint32_t x;
} tdata;

tdata thread_data[NUM_THREADS];

void *sender(void* tdata);
void *receiver(void* tdata);

#define WRONG wrong(__LINE__);

void wrong(unsigned line)
{
  mca_status_t status;
  mcapi_finalize(&status);
  fprintf(stderr,"FAIL: line=%u\n", line);
  fflush(stdout);
  exit(1);
}


int main(void)
{
  int i;
  pthread_t threads[NUM_THREADS];
  int rc = 0;

  mcapi_set_debug_level(6);

  // test receiver first and then sender
  printf("Test 1\n");
  rc += pthread_create(&threads[0], NULL, receiver,(void*)&thread_data[0]);
  rc += pthread_create(&threads[1], NULL, sender,(void*)&thread_data[1]);

  // sync up
  for (i = 0; i < 2; i++) {
    pthread_join(threads[i],NULL);
  }

  // now test sender first and then receiver
  printf("Test 2\n");
  rc += pthread_create(&threads[0], NULL, sender,(void*)&thread_data[0]);
  rc += pthread_create(&threads[1], NULL, receiver,(void*)&thread_data[1]);

  // sync up
  for (i = 0; i < 2; i++) {
    pthread_join(threads[i],NULL);
  }

  if (rc == 0) {
    printf("   Test PASSED\n");
  }
  return rc;
}

void *sender(void* data)
{
  char buffer[NUM_MSGS];
  int count = 0;

  mca_status_t status;
  int i;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_priority_t priority = 1;
  mcapi_endpoint_t remote_recv_endpt;
 
  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    WRONG;
  }
  
  
  /* make send endpoint */
  send_endpt = mcapi_endpoint_create(0,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to create send endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff))); 
    WRONG;
  }
  
  
  /* get a recv endpoint */
  remote_recv_endpt = mcapi_endpoint_get(DOMAIN,NODE+1,1,MCA_INFINITE,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to get receive endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff))); 
    WRONG;
  }
  
  /* do the sends */
  for (count = 0; count < NUM_MSGS; count++) {
    sprintf(buffer,"Sending: %d",count);
    
    printf("Sending: [%s]\n",buffer);
    
    mcapi_msg_send(send_endpt,
                   remote_recv_endpt,
                   buffer,
                   strlen(buffer),
                   priority,
                   &status);
    
    if ((status != MCAPI_SUCCESS) && (TIMEOUT)) {
      /* yield and then retry */
      for (i = 0; i < TIMEOUT; i++) {
        if (status == MCAPI_ERR_MEM_LIMIT) {
            fprintf(stderr,"WARNING: Send failed: reason:%s  send_count:%d.  Will yield and re-try.\n",mcapi_display_status(status,status_buff,sizeof(status_buff)),count);
            sched_yield();
            mcapi_msg_send(send_endpt,
                           remote_recv_endpt,
                           buffer,
                           strlen(buffer),
                           priority,
                           &status);
        }
        if (status == MCAPI_SUCCESS) {
          break;
        }
      }
    }
    if (status != MCAPI_SUCCESS) {
      fprintf(stderr,"\nFAIL: Send failed: reason:%s  send_count:%d\n",mcapi_display_status(status,status_buff,sizeof(status_buff)),count);
      WRONG;
    }
  }
     
  /* finalize */
  mcapi_finalize(&status);
  
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nFAIL: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));   
    WRONG;
  }
  
  return NULL;
}

void *receiver(void* data)
{  
  char buffer[NUM_MSGS];
  int count = 0;
  size_t recv_size;
  mca_status_t status;
 
  int i;
mcapi_param_t parms;
  mcapi_info_t version;

  /* create a node */
  mcapi_initialize(DOMAIN,NODE+1,NULL,&parms,&version,&status);
  
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nFAIL: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));   
    WRONG;
  }
  
  
  /* make recv endpoint */
  recv_endpt = mcapi_endpoint_create(1,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nFAIL: Failed to create recv endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff))); 
    WRONG;
  }
  
  
  
  /* do the receives */
  for (count = 0; count < NUM_MSGS; count++) {
      mcapi_msg_recv(recv_endpt,
                     buffer,
                     sizeof(buffer),
                     &recv_size,
                     &status);
      if ((status != MCAPI_SUCCESS) && (TIMEOUT)) {
        /* yield and then retry */
        for (i = 0; i < TIMEOUT; i++) {
          if (status != MCAPI_SUCCESS) {
            fprintf(stderr,"\nWARNING: Recv failed: reason:%s  recv_count:%d.  Will yield and re-try.\n",mcapi_display_status(status,status_buff,sizeof(status_buff)),count);
            sched_yield();
            mcapi_msg_recv(recv_endpt,
                           buffer,
                           sizeof(buffer),
                           &recv_size,
                           &status);
          }
          if (status == MCAPI_SUCCESS) {
            break;
          }
        }
      }
      
      if (status == MCAPI_SUCCESS) {
        printf("count=%d, Received: [%s]\n",count,buffer);
      } else {
        fprintf(stderr,"\nFAIL: Recv failed: reason:%s  recv_count:%d.\n",mcapi_display_status(status,status_buff,sizeof(status_buff)),count);
        WRONG;
      }
  }

  /* finalize */
  mcapi_finalize(&status);
  
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nFAIL: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));   
    WRONG;
  }

  return NULL;
}

