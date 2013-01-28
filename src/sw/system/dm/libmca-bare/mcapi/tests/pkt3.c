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
/* Test: pkt3
   Description: Tests simple in-order non-blocking pkt_channel_send and 
   pkt_channel_recv calls between endpoints on a single node. Doesn't test
   any error conditions - expects all successes.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <assert.h>
#include <mca_config.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define PORT_NUM1 100
#define PORT_NUM2 200
#define PORT_NUM3 300
#define PORT_NUM4 400


#define BUFF_SIZE 64

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

void* buffers [MCAPI_MAX_BUFFERS+1];
int num_elements = 0;

char* msg [10] = {"zero","one","two","three","four","five","six","seven","eight","nine"};

void send (mcapi_pktchan_send_hndl_t send_handle, mcapi_endpoint_t recv,char* msg,mca_status_t status,int exp_status) {
  mcapi_request_t request;
  int size = strlen(msg);
  size_t sent_size;
do {
    mcapi_pktchan_send_i(send_handle,msg,size,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&sent_size,&status)) {} /* these requests should complete right away */
  if (status != exp_status) { WRONG}
  if (size != sent_size) {
    fprintf(stderr,"Error: expected_size=%i,received_size=%i\n",(int)size,(int)sent_size);
  }
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has sent: [%s]\n",(int)send_handle,msg);
  }
}

void recv (mcapi_pktchan_recv_hndl_t recv_handle,mca_status_t status,int exp_status,char* exp_data) {
  size_t recv_size;
  mcapi_request_t request;
  char *buffer;
  int free_buffer = 1;
do {
    mcapi_pktchan_recv_i(recv_handle,(void **)((void*)&buffer),&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&recv_size,&status)) {} /* these requests should complete right away */
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has received %i bytes: [%s]\n",(int)recv_handle,(int)recv_size,buffer);
    if (strcmp(exp_data,buffer)) {
      fprintf(stderr,"expected [%s], but received: [%s]\n",exp_data,buffer);
      WRONG;
    }
    if (free_buffer) {
      mcapi_pktchan_release((void *)buffer,&status);
      if (status != MCAPI_SUCCESS) { WRONG} 
    } else {
      num_elements++;
      buffers[num_elements-1] = buffer;
      assert(num_elements < (MCAPI_MAX_BUFFERS+1));
    }

  }
}

int main () {
  mca_status_t status;
  mcapi_request_t request;
  size_t size;
  mcapi_endpoint_t ep1,ep2,ep3,ep4,ep5,ep6,ep7,ep8;
  /* cases:
     1: both named endpoints (1,2)
     2: both anonymous endpoints (3,4)
     3: anonymous sender, named receiver (5,6)
     4: anonymous receiver, named sender (7,8)
  */
  mcapi_pktchan_send_hndl_t s1,s2,s3,s4; /* s1 = ep1->ep2, s2 = ep3->ep4, s3 = ep5->ep6, s4 = ep7->ep8 */
  mcapi_pktchan_recv_hndl_t r1,r2,r3,r4; /* r1 = ep1->ep2, r2 = ep3->ep4, r3 = ep5->ep6, r4 = ep7->ep8 */
  
  int i;
mcapi_param_t parms;
  mcapi_info_t version;

  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    buffers[i] = NULL;
  }  
  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
    
  /* create endpoints */
  ep1 = mcapi_endpoint_create(PORT_NUM1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(PORT_NUM2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep3 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG}

  ep4 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG}

  ep5 = mcapi_endpoint_create(PORT_NUM3,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep6 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
   
  ep7 = mcapi_endpoint_create(PORT_NUM4,&status);
    if (status != MCAPI_SUCCESS) { WRONG }
    
  ep8 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  /*************************** connect the channels *********************/
do {
    mcapi_pktchan_connect_i(ep1,ep2,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_connect_i(ep3,ep4, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_connect_i(ep5,ep6, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_connect_i(ep7,ep8, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG } 


  /*************************** open the channels *********************/ 
  /* now open the channels */
do {
    mcapi_pktchan_recv_open_i(&r1 /*recv_handle*/,ep2, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_recv_open_i(&r2 /*recv_handle*/,ep4, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_recv_open_i(&r3 /*recv_handle*/,ep6, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_recv_open_i(&r4 /*recv_handle*/,ep8, &request, &status);
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

do {
    mcapi_pktchan_send_open_i(&s2 /*send_handle*/,ep3, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_send_open_i(&s3 /*send_handle*/,ep5, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_send_open_i(&s4 /*send_handle*/,ep7, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  /* send 10 messages on each send handle */
  for (i = 0; i < 10; i++) {
    send (s1,ep2,msg[i],status,MCAPI_SUCCESS);
    send (s2,ep4,msg[i],status,MCAPI_SUCCESS);
    send (s3,ep6,msg[i],status,MCAPI_SUCCESS);
    send (s4,ep8,msg[i],status,MCAPI_SUCCESS);
  }

  /* receive 10 messages on each recv handle */
  for (i = 0; i < 10; i++) {
    recv (r1,status,MCAPI_SUCCESS,msg[i]);
    recv (r2,status,MCAPI_SUCCESS,msg[i]);
    recv (r3,status,MCAPI_SUCCESS,msg[i]);
    recv (r4,status,MCAPI_SUCCESS,msg[i]);
  }
 
  /* interleave sends followed by receives */
  for (i = 0; i < 10; i++) {
    send (s1,ep2,msg[i],status,MCAPI_SUCCESS);
    recv (r1,status,MCAPI_SUCCESS,msg[i]);
  }

  /* close the channels */
do {
    mcapi_pktchan_recv_close_i(r1,&request,&status);     
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_recv_close_i(r2,&request,&status);    
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_recv_close_i(r3,&request,&status);     
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_pktchan_recv_close_i(r4,&request,&status);    
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

do {
    mcapi_pktchan_send_close_i(s2,&request,&status);  
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_send_close_i(s3,&request,&status); 
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_pktchan_send_close_i(s4,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  mcapi_finalize(&status);
  printf("   Test PASSED\n");
  return 0;
}
