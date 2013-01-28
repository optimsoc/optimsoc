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
/* Test: msg14
   Description: This is a bug that was found in the fifo queue management by a pthreads test.
        3 elements in Q
        push
        push
        pop
        pop
        pop
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

#define BUFF_SIZE 64

#define NUM_MSGS 500

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

typedef struct {
  char buffer[BUFF_SIZE];
  mcapi_request_t request;
  mca_status_t status;
} msg;

msg sent_msgs[NUM_MSGS];
msg recvd_msgs[NUM_MSGS];



void send (mcapi_endpoint_t send,mcapi_endpoint_t recv) {
  static int i = 0;
  int priority = 0;
  size_t size = 0;

  sprintf(sent_msgs[i].buffer,"%i",i);
  size = strlen(sent_msgs[i].buffer);
  do {
    mcapi_msg_send_i(send,recv,sent_msgs[i].buffer,size,priority,&sent_msgs[i].request,&sent_msgs[i].status);
  } while (sent_msgs[i].status == MCAPI_ERR_REQUEST_LIMIT);

  mcapi_wait (&sent_msgs[i].request,&size,TIMEOUT,&sent_msgs[i].status);
  if (sent_msgs[i].status == MCAPI_SUCCESS) {
    printf("endpoint=%i has sent: [%s]\n",(int)send,sent_msgs[i].buffer);
  } else {
    WRONG
  }
  i++;
}

void recv (mcapi_endpoint_t recv) {
  static int i = 0;
  size_t recvd_size = 0;

  do {
    mcapi_msg_recv_i(recv,recvd_msgs[i].buffer,BUFF_SIZE,&recvd_msgs[i].request,&recvd_msgs[i].status);
  } while (recvd_msgs[i].status == MCAPI_ERR_REQUEST_LIMIT);
  mcapi_wait (&recvd_msgs[i].request,&recvd_size,TIMEOUT,&recvd_msgs[i].status);

  if (recvd_msgs[i].status == MCAPI_SUCCESS) {
    printf("endpoint=%i has received: [%s]\n",(int)recv,recvd_msgs[i].buffer);
  } else {
    WRONG
  }
  if (strcmp(recvd_msgs[i].buffer,sent_msgs[i].buffer)) {
    printf("endpoint=%i has received: [%s] but expected [%s]\n",(int)recv,recvd_msgs[i].buffer,sent_msgs[i].buffer);
    WRONG
  }
  i++;
}


int main () {
  mca_status_t status;
mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_endpoint_t ep1,ep2;

  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
    
  /* create endpoints */
  ep1 = mcapi_endpoint_create(PORT_NUM1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(PORT_NUM2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }


  /* this sequence of events is from grepping for "internal" in the saved output from the failing test */
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  recv(ep2);
  recv(ep2);
  
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  send (ep1,ep2);
  recv(ep2);
  
  
  mcapi_finalize(&status);
  printf("   Test PASSED\n");
  return 0;
}
