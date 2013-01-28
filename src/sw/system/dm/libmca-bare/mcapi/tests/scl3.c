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
/* Test: scl3

*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* for exit */
#include <mca_config.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


typedef struct {
  mca_status_t status;
  int buffer;
} entry;

entry send_requests[MCAPI_MAX_QUEUE_ELEMENTS+1];
entry recv_requests[MCAPI_MAX_QUEUE_ELEMENTS+1];



mcapi_boolean_t sender( mcapi_sclchan_send_hndl_t send_handle,mca_status_t exp_status);
mcapi_boolean_t receiver( mcapi_sclchan_recv_hndl_t recv_handle,mca_status_t exp_status);
mcapi_boolean_t check_results();


int fail () {
  mca_status_t status;
  printf(" Test FAILED\n");
  mcapi_finalize(&status);
  exit(1);
}

int main(int argc, char** argv)
{

  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;
  mca_status_t status;
  mcapi_request_t request;
  int i;

  mcapi_sclchan_recv_hndl_t recv_handle; /* r1 = ep1->ep2 */
  mcapi_sclchan_send_hndl_t send_handle; /* r1 = ep1->ep2 */
  size_t size;
mcapi_param_t parms;
  mcapi_info_t version;

  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    return -1;
  }
  
  /* create the endpoints */
  send_endpt = mcapi_endpoint_create(1,&status);
  recv_endpt =  mcapi_endpoint_create(2,&status);

  /* connect the channel */

  /* connect the channel */
do {
    mcapi_sclchan_connect_i(send_endpt,recv_endpt,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status == MCAPI_ERR_CHAN_CONNECTED) {
    fprintf(stderr,"\nokay, already connected: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
  }
  else if (status != MCAPI_SUCCESS) { 
    fprintf(stderr,"\nERROR: Failed to connect: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    fail();
  }


  /* open the endpoint handles */
do {
    mcapi_sclchan_send_open_i(&send_handle /*send_handle*/,send_endpt, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) { 
    fprintf(stderr,"\nERROR: Failed to open send handle: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    fail();
  }
do {
    mcapi_sclchan_recv_open_i(&recv_handle /*recv_handle*/,recv_endpt, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) { 
    fprintf(stderr,"\nERROR: Failed to open recv handle: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    fail();
  }

  /* issue sends */
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    if (!sender(send_handle,MCAPI_SUCCESS)) {
      fail();
    }
  }

  /* queue should be full */
  if (!sender(send_handle,MCAPI_ERR_MEM_LIMIT)) {
    fail();
  }

  /* issue receives */
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    if (!receiver(recv_handle,MCAPI_SUCCESS)) {
      fail();
    }
  }

  /* check results */
  if (! check_results()) {
    fail();
  }

  /* close handles */


  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    fail();
  }
  
  printf("   Test PASSED\n");
  return 0;
}

mcapi_boolean_t sender( mcapi_sclchan_send_hndl_t send_handle,mca_status_t exp_status)
{
  
  static int i = 0;
  send_requests[i].buffer = i;
  printf("Sending: %d\n",i);
  mcapi_sclchan_send_uint32(send_handle,
                   send_requests[i].buffer,
                   &send_requests[i].status);
  if (send_requests[i].status != exp_status) {
    fprintf(stderr,"FAIL: send status=%s (expected %s)\n",
            mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)),
            mcapi_display_status(exp_status,status_buff,sizeof(status_buff)));
    return MCAPI_FALSE;
  }
  i++;
  return MCAPI_TRUE;
}


mcapi_boolean_t receiver( mcapi_sclchan_recv_hndl_t recv_handle,mca_status_t exp_status)
{  

  static int i = 0;
  recv_requests[i].buffer = mcapi_sclchan_recv_uint32(recv_handle,
                   &recv_requests[i].status );
  printf("Received: %d\n",recv_requests[i].buffer);
  if (recv_requests[i].status != exp_status) {
    fprintf(stderr,"FAIL: recv status=%s (expected %s)\n",
            mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff)),
            mcapi_display_status(exp_status,status_buff,sizeof(status_buff)));
    return MCAPI_FALSE;
  }
  i++;
  return MCAPI_TRUE;
}

mcapi_boolean_t check_results () 
{
  int i;


  /* test that the data sent matches the data received */
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    if ((send_requests[i].status != MCAPI_SUCCESS) ||
        (recv_requests[i].status != MCAPI_SUCCESS) ) {
      fprintf(stderr,"FAIL: send status=%s received status=%s (expected MCAPI_SUCCESS for both)\n",
              mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)),
              mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff)));
      return MCAPI_FALSE;
    }

    if (send_requests[i].buffer != recv_requests[i].buffer) {
      fprintf(stderr,"FAIL: send data[%d] != recv data[%d]\n",send_requests[i].buffer,recv_requests[i].buffer);
      return MCAPI_FALSE;
    }
    printf("success: sent: [%d], recvd: [%d]\n",send_requests[i].buffer,recv_requests[i].buffer);
  }
  return MCAPI_TRUE;
}



