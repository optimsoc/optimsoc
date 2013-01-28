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
/* Test: pkt4
   Description: Tests simple out-of-order non-blocking pkt_channel_send and 
   pkt_channel_recv calls to two endpoints on asingle node.  This is a 
   directed test for a bug found using a pthreaded test.  Specifically this 
   test makes sure that pktchan calls in the order: recv send send recv obey FIFO.
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

#define NUM_REQUESTS 4

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

char send_buf[256];
typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char *buffer;
  size_t size;
} r_entry;
r_entry requests[NUM_REQUESTS];

const int fail = 1;
const int ok = 0;

int check_results() {
  int i;
  /* gather up all the requests */
  for (i = 0; i < NUM_REQUESTS; i++) {
    if (!mcapi_test(&requests[i].request,&requests[i].size,&requests[i].status)) {
      printf("found incomplete requests %i\n",i); 
      return fail;
    }
    if (requests[i].status != MCAPI_SUCCESS) { 
      printf("fail: status = %s\n",mcapi_display_status(requests[i].status,status_buff,sizeof(status_buff)));
      return fail;
    }
  }
  
  /* compare the data received */
  if (strcmp(requests[0].buffer,"one")) {
    printf("fail: buffers[%i]%s == \"one\"\n",0,requests[0].buffer);
    return fail;
  }
  
  if (strcmp(requests[3].buffer,"two")) {
    printf("fail: buffers[%i]%s == \"two\"\n",2,requests[2].buffer);
    return fail;
  }
  
  return ok;
}


int main () {
  size_t size;
  mcapi_endpoint_t ep1,ep2;
  mcapi_pktchan_send_hndl_t s1; /* s1 = ep1->ep2 */
  mcapi_pktchan_recv_hndl_t r1; /* r1 = ep1->ep2 */
  int i = 0;
  int rc = 1;
  mca_status_t status;
  mcapi_request_t request;
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
 
  /*************************** connect the channels *********************/
do {
    mcapi_pktchan_connect_i(ep1,ep2,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  /*************************** open the channels *********************/
do {
    mcapi_pktchan_recv_open_i(&r1 /*recv_handle*/,ep2, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_send_open_i(&s1 /*send_handle*/,ep1, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  /* tests: recv send send recv obeys FIFO */
do {
    mcapi_pktchan_recv_i(r1,(void **)((void*)&requests[i].buffer),&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_pktchan_send_i(s1,"one",sizeof("one"),&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_pktchan_send_i(s1,"two",sizeof("two"),&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
do {
    mcapi_pktchan_recv_i(r1,(void **)((void*)&requests[i].buffer),&requests[i].request,&requests[i].status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
 
  rc = check_results();

  /* close the channels */
do {
    mcapi_pktchan_recv_close_i(r1,&request,&status);     
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
                                                                                                                                      
do {
    mcapi_pktchan_send_close_i(s1,&request,&status); 
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  mcapi_finalize(&status);
  if (rc == 0) {
    printf("   Test PASSED\n");
  } else {
    printf("   Test FAILED\n");
  }
  return rc;
}
