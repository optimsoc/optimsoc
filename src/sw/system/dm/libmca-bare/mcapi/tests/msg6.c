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
/* Test: msg6
   Description: Tests non-blocking message send and receive between endpoints on a 
   single node.  Sends NUM_MSGS messages, then receives NUM_MSGS messages, then receives NUM_MSGS messages, 
   and finally sends NUM_MSGS */

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <string.h>
#include <mca_config.h>


#define BUFF_SIZE MCAPI_MAX_MSG_SIZE 



#define ITERATIONS 2
#define NUM_MSGS 10

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} request_entry;

request_entry send_requests[NUM_MSGS*ITERATIONS];
request_entry recv_requests[NUM_MSGS*ITERATIONS];

const int fail = 1;
const int ok = 0;

void sender(mcapi_endpoint_t send_endpt,mcapi_endpoint_t recv_endpt,int start,int end);
void receiver(mcapi_endpoint_t recv_endpt,int start,int end);
int check_results();

int main(void)
{

  
  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;
  mca_status_t status;
  int rc;
  mcapi_param_t parms;
  mcapi_info_t version;

  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    return fail;
  }
  
  /* create the endpoints */
  send_endpt = mcapi_endpoint_create(1,&status);
  recv_endpt =  mcapi_endpoint_create(2,&status);

  sender(send_endpt,recv_endpt,0,NUM_MSGS-1);
  receiver(recv_endpt,0,NUM_MSGS-1);
 
  receiver(recv_endpt,NUM_MSGS,(NUM_MSGS*2)-1);
  sender(send_endpt,recv_endpt,NUM_MSGS,(NUM_MSGS*2)-1);

  rc = check_results();

  mcapi_finalize(&status);

  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    return fail;
  }
  
  if (rc == 0) { printf("   Test PASSED\n"); } else { printf("   Test FAILED\n"); }
  return rc;
}

void sender(mcapi_endpoint_t send_endpt,mcapi_endpoint_t recv_endpt,int start,int end)
{
  int i;
  size_t size;
  mcapi_priority_t priority = 1;
  for (i=start; i <= end; i++) {
    sprintf(send_requests[i].buffer,"Sending: %d",i);
    send_requests[i].size=strlen(send_requests[i].buffer);
    if (start > 0) {printf ("sent: index=%i data=%s **",i,send_requests[i].buffer);}
    send_requests[i].status = MCAPI_ERR_MEM_LIMIT;
    // retry if we run out of memory/buffer entries
    while ( send_requests[i].status == MCAPI_ERR_MEM_LIMIT) {
do {
        mcapi_msg_send_i(send_endpt,recv_endpt,send_requests[i].buffer,send_requests[i].size,priority, &send_requests[i].request,&send_requests[i].status); 
//retry if all request handles are in-use
} while (send_requests[i].status == MCAPI_ERR_REQUEST_LIMIT);
      mcapi_wait(&send_requests[i].request,&size,0xffff,&send_requests[i].status);
      if (send_requests[i].status != MCAPI_SUCCESS) {
        fprintf(stderr, "sender ERROR: status=%s\n",mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)));
      }
    }
  }
}


void receiver(mcapi_endpoint_t recv_endpt,int start,int end)
{  
  int i;
  for (i=start; i <= end; i++) {
    if (start > 0) { printf ("recv: index=%i  **",i); }
do {
      mcapi_msg_recv_i(recv_endpt,recv_requests[i].buffer,sizeof(recv_requests[i].buffer),&recv_requests[i].request,&recv_requests[i].status );
//retry if all request handles are in-use
} while (recv_requests[i].status == MCAPI_ERR_REQUEST_LIMIT);

  }   
}

int check_results () 
{
  int i;
  
  /* gather up all the receives */
  for (i = 0; i < NUM_MSGS*ITERATIONS; i++) {
    recv_requests[i].size = sizeof (recv_requests[i].buffer);
    mcapi_test(&recv_requests[i].request,&recv_requests[i].size,&recv_requests[i].status);
    //if (recv_requests[i].status == MCAPI_INCOMPLETE) {
    //  fprintf(stderr,"FAIL: found incomplete receive request\n");
    //  return fail; /* since the sends have completed, the receives should now be complete */
    //}
    if (recv_requests[i].size !=  send_requests[i].size) {
      fprintf(stderr,"\nFAIL: recv[%i]: size returned by test[%i] != sent size[%i], status = %s\n",
              i,(int)recv_requests[i].size,(int)send_requests[i].size,mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff))); 
   
      //return fail;
    }
  }

  /* test that the data sent matches the data received */
  for (i = 0; i < NUM_MSGS*ITERATIONS; i++) {
    if ((send_requests[i].status != MCAPI_SUCCESS) ||
        (recv_requests[i].status != MCAPI_SUCCESS) ) {
      fprintf(stderr,"FAIL: send status=%s received status=%s (expected MCAPI_SUCCESS for both)\n",
              mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)),
              mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff)));
      return fail;
    }
    if ((send_requests[i].size != recv_requests[i].size) ||
        (send_requests[i].size == 0) || (recv_requests[i].size == 0)){
      fprintf(stderr,"FAIL: send size=%i,recv size=%i\n",(int)send_requests[i].size,(int)recv_requests[i].size);
      return fail;
    }
    if (strcmp(send_requests[i].buffer,recv_requests[i].buffer)) {
      fprintf(stderr,"FAIL: send data[%s] != recv data[%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
      return fail;
    }
    printf("success: sent: [%s], recvd: [%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
  }
  return ok;
}



