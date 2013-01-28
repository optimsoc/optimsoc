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
  Test: msg5
  Description: This is a pthreads test that sends 30 messages using a sender thread 
  and a receiver thread.  This test is run twice, once with the receiver first and once with the 
  sender first.  Only 30 messages are sent so queue overflow is not tested in this test.
  Tests non-blocking send/recv.
*/



#include <mcapi.h>

char status_buff1[MCAPI_MAX_STATUS_SIZE];
char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif



#define my_timeout 200 

static int ok = 1;
static int fail = -1;

#define NUM_MSGS 30 
#ifndef BUFF_SIZE
#define BUFF_SIZE 64
#endif

typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} request_entry;

request_entry send_requests[NUM_MSGS];
request_entry recv_requests[NUM_MSGS];

typedef struct {
  mcapi_boolean_t done;
} thread_data_t;

void* sender(void*);
void* receiver(void*);
int check_results();

int main(void)
{
  int i;
  pthread_t threads[2];
  thread_data_t tdata[2];

  mcapi_set_debug_level(6);

  tdata[0].done = MCAPI_FALSE;
  tdata[1].done = MCAPI_FALSE;
  
  // test receiver first and then sender
  printf("Test 1\n");
  pthread_create(&threads[0], NULL, receiver, (void*)&tdata[0]);
  pthread_create(&threads[1], NULL, sender, (void*)&tdata[1]);
  for (i = 0; i < 2; i++) {
    pthread_join(threads[i],NULL);
  }

  if ( check_results() != ok) {
    return 1;
  }

  // now test sender first and then receiver
  printf("Test 2\n");
  pthread_create(&threads[0], NULL, sender, (void*)&tdata[0]);
  pthread_create(&threads[1], NULL, receiver,(void*) &tdata[1]);

  // join
  for (i = 0; i < 2; i++) {
    pthread_join(threads[i],NULL);
  }
  
  if ( check_results() != ok) {
    return 1;
  }
  printf("   Test PASSED\n");
  return 0; 
}

void* sender(void *tdata)
{
  int count = 0;
  //  thread_data_t* p_tdata = (thread_data_t*) tdata;
  int node_num = NODE+1;
  int port_num = 1;
  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;
  mca_status_t status;
  int n,i;
  size_t size;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_priority_t priority = 1;
  int num_requests = 0;
  
  /* create a node */
  mcapi_initialize(DOMAIN,node_num,NULL,&parms,&version,&status);
  printf("initialize node_num=%i, status=%s\n",node_num,mcapi_display_status(status,status_buff,sizeof(status_buff)));
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR sender: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    exit(-1);
  }
  
  n = mcapi_node_id_get (&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR sender: Failed to get node_num. status=[%s]\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }
  if (node_num != n) {
    fprintf(stderr,"\nERROR sender: Failed expected node_num=[%i], received node_num=[%i], status=%s\n",(int)node_num,(int)n,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }
  
  
  /* make send endpoint */
  send_endpt = mcapi_endpoint_create(port_num,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR sender: Failed to create send endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }
  
  /* make sure the receiver exists */
  recv_endpt = mcapi_endpoint_get (DOMAIN,NODE+2,port_num,MCA_INFINITE,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR sender: Failed to get the recv endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }
  
  for (count = 0; count < NUM_MSGS; count++) {
    i = num_requests++;
    sprintf(send_requests[i].buffer,"Sending %i",count);
    printf("\nSending: [%s]",send_requests[i].buffer);
    send_requests[i].size=strlen(send_requests[i].buffer);
    do {
      mcapi_msg_send_i(send_endpt,recv_endpt,send_requests[i].buffer,send_requests[i].size,priority,&send_requests[i].request,&send_requests[i].status);
      //retry if all request handles are in-use
    } while (status == MCAPI_ERR_REQUEST_LIMIT);
  }
  
  /* gather up all the sends (not blocking this thread)*/
  for (i = 0; i < NUM_MSGS; i++) {
    /* we can use wait without hitting a deadlock because these sends will complete quickly */
    mcapi_wait(&send_requests[i].request,&size,TIMEOUT,&send_requests[i].status);
    if (size !=  send_requests[i].size) {
      fprintf(stderr,"ERROR sender: size returned by wait[%i] != sent size[%i]\n",
              (int)size,(int)send_requests[i].size); 
      exit(-1);
    }
  }
    
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR sender: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    exit(-1);
  }

  return &ok;
}

void* receiver(void *tdata)
{  
  int count = 0;
  int node_num = NODE+2;
  int port_num = 1;
  //  thread_data_t* p_tdata = (thread_data_t*) tdata;
  int n,i;
  mca_status_t status;
  mcapi_endpoint_t recv_endpt;
  mcapi_param_t parms;
  mcapi_info_t version;
  int num_requests = 0;

  
  /* create a node */
  mcapi_initialize(DOMAIN,node_num,NULL,&parms,&version,&status);
  printf("initialize node_num=%i, status=%s\n",node_num,mcapi_display_status(status,status_buff,sizeof(status_buff)));
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR receiver: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    exit(-1);
  }
  
  n = mcapi_node_id_get (&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR receiver: Failed to get node_num.status=[%s]\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }
  if (node_num != n) {
    fprintf(stderr,"\nERROR receiver: Failed expected node_num=[%i], received node_num=[%i], status=%s\n",(int)node_num,(int)n,mcapi_display_status(status,status_buff,sizeof(status_buff)));
      exit(-1);
  }
    
  /* make recv endpoint */
  recv_endpt = mcapi_endpoint_create(port_num,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR receiver: Failed to create recv endpoint: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    exit(-1);
  }

  
  for (count = 0; count < NUM_MSGS; count++) {
    i = num_requests++; 
    printf("\nReceiving %i",i);
    do {
      mcapi_msg_recv_i(recv_endpt,recv_requests[i].buffer,sizeof(recv_requests[i].buffer),&recv_requests[i].request,&recv_requests[i].status );
      if (recv_requests[i].status != MCAPI_SUCCESS) { printf ("ERROR: mcapi_msg_recv_i failed status i=%d status=%s\n",i,mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff))); }
      
      //retry if all request handles are in-use
    } while (status == MCAPI_ERR_REQUEST_LIMIT); 
  }
  
  /* gather up all the receives (blocking this thread) */
  for (i = 0; i < NUM_MSGS; i++) {
    mcapi_wait(&recv_requests[i].request,&recv_requests[i].size,TIMEOUT,&recv_requests[i].status);
    if (recv_requests[i].status != MCAPI_SUCCESS) { printf ("ERROR: receiver status i=%d status=%s\n",i,mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff))); }
  }
  
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR receiver: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    exit(-1);
  }
  
  return &ok;
}

int check_results () 
{

  int i;
  int rc = ok;

  /* test that the data sent matches the data received */
  for (i = 0; i < NUM_MSGS; i++) {
    if ((send_requests[i].status != MCAPI_SUCCESS) ||
        (recv_requests[i].status != MCAPI_SUCCESS) ) {
      fprintf(stderr,"Error: i=%d send status=%s received status=%s (expected MCAPI_SUCCESS for both)\n",i,
              mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)),
              mcapi_display_status(recv_requests[i].status,status_buff1,sizeof(status_buff1)));
      rc = fail;
    }
    else if ((send_requests[i].size != recv_requests[i].size) ||
             (send_requests[i].size == 0) || (recv_requests[i].size == 0)){
      fprintf(stderr,"Error: send size=%i,recv size=%i\n",(int)send_requests[i].size,(int)recv_requests[i].size);
      rc = fail;
    }
    else if (strcmp(send_requests[i].buffer,recv_requests[i].buffer)) {
      fprintf(stderr,"Error: send data[%s] != recv data[%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
      rc = fail;
    }
  }
  
  return rc;
}
