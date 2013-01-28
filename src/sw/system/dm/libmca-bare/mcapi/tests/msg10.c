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
/* Test: msg9
   Description: This test was written to reproduce a fail found by the random msg7 test.  It tests
   a specific order of simple out-of-order non-blocking msg_send/msg_recv calls between endpoints
   on a single node. It makes sure a specific order of msg_send/msg_recv obeys FIFO.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>


#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define PORT_NUM1 100
#define PORT_NUM2 200

#define BUFF_SIZE 256

#define MAX_NUM_REQUESTS 20

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} r_entry;

r_entry requests[MAX_NUM_REQUESTS];

mcapi_boolean_t check_results(int num) {
  int i;
  /* gather up all the requests */
  for (i = 0; i < num; i++) {
    if (!mcapi_test(&requests[i].request,&requests[i].size,&requests[i].status)) {
      printf("found incomplete request at index=%i\n",i); 
      return MCAPI_FALSE;
    }
    if (requests[i].status != MCAPI_SUCCESS) { 
      printf("FAIL: status = %s\n",mcapi_display_status(requests[i].status,status_buff,sizeof(status_buff)));
      return MCAPI_FALSE;
    }
  }
  
  /* compare the data received */
  i = 2;
  if (strcmp(requests[i].buffer,"one")) {
    printf("FAIL: buffers[%i]%s == \"one\"\n",i,requests[i].buffer);
    return MCAPI_FALSE;
  }
  i = 3;
  if (strcmp(requests[i].buffer,"two")) {
    printf("FAIL: buffers[%i]%s == \"two\"\n",i,requests[i].buffer);
    return MCAPI_FALSE;
  }
  i = 4;
  if (strcmp(requests[i].buffer,"three")) {
    printf("FAIL: buffers[%i]%s == \"three\"\n",i,requests[i].buffer);
    return MCAPI_FALSE;
  }
  i = 7;
  if (strcmp(requests[i].buffer,"four")) {
    printf("FAIL: buffers[%i]%s == \"four\"\n",i,requests[i].buffer);
    return MCAPI_FALSE;
  }
  i = 8;
  if (strcmp(requests[i].buffer,"five")) {
    printf("FAIL: buffers[%i]%s == \"five\"\n",i,requests[i].buffer);
    return MCAPI_FALSE;
  }
  return MCAPI_TRUE;
}


int main () {
 
  mcapi_endpoint_t ep1,ep2;
  mcapi_priority_t priority = 1;

  int i = 0;
  int rc = 1;
  mca_status_t status;
mcapi_param_t parms;
  mcapi_info_t version;

  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
    
  /* create endpoints */
  ep1 = mcapi_endpoint_create(PORT_NUM1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(PORT_NUM2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
 
  i = 0;
  /* tests: 2 sends 3 recvs 2 sends 1 recv (fragments queue) */
do {
    mcapi_msg_send_i(ep1,ep2,"one",sizeof("one"),priority,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_send_i(ep1,ep2,"two",sizeof("two"),priority,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_recv_i(ep2,requests[i].buffer,BUFF_SIZE,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_recv_i(ep2,requests[i].buffer,BUFF_SIZE,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_recv_i(ep2,requests[i].buffer,BUFF_SIZE,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_send_i(ep1,ep2,"three",sizeof("three"),priority,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_send_i(ep1,ep2,"four",sizeof("four"),priority,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
  /* this receive completes immediately, fragmenting the queue */
do {
    mcapi_msg_recv_i(ep2,requests[i].buffer,BUFF_SIZE,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
  /* this receive should just set a reservation */
do {
    mcapi_msg_recv_i(ep2,requests[i].buffer,BUFF_SIZE,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_msg_send_i(ep1,ep2,"five",sizeof("five"),priority,&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;

  if (check_results(i)) {
    rc = 0;
  }

  mcapi_finalize(&status);
  if (rc == 0) {
    printf("   Test PASSED\n");
  } else {
    printf("   Test FAILED\n");
  }
  return rc;
}
