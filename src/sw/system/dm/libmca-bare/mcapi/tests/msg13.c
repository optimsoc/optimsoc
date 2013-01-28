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
/* Test: msg13
   Description: Tests simple blocking msgsend and msgrecv calls to several endpoints
   on a single node.  Maxes sure we can send 256 bytes. 
*/

#include <mcapi.h>
#include <mca_config.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mcapi_impl_spec.h>
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


#define MCAPI_MAX_ECHO_SIZE 1024
#define ECHO_BUF_SIZE MCAPI_MAX_ECHO_SIZE + 1


#define WRONG wrong(__LINE__,status);
void wrong(unsigned line,mca_status_t status)
{
  char status_buff[128];
  fprintf(stderr,"WRONG: line=%u %s\n", line,mcapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

void send (mcapi_endpoint_t send, mcapi_endpoint_t recv,char* msg,mca_status_t status,int exp_status) {
  int size = strlen(msg);
  mcapi_priority_t priority = 1;
  printf("sending size=%d bytes",size);
  mcapi_msg_send(send,recv,msg,size,priority,&status);
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("endpoint=%i has sent: [%s]\n",(int)send,msg);
  }
}

void recv (mcapi_endpoint_t recv,mca_status_t status,int exp_status,char* buffer) {
  size_t recv_size;
  mcapi_msg_recv(recv,buffer,ECHO_BUF_SIZE,&recv_size,&status);
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("endpoint=%i has received: [%s]\n",(int)recv,buffer);
  }
}

int main () {
  mca_status_t status;
  mcapi_endpoint_t ep1,ep2;
  int rc = 0;
  char msgbuf[ECHO_BUF_SIZE] = "";
  int op_size = MCAPI_MAX_MSG_SIZE; 
  char recvbuf[ECHO_BUF_SIZE] = "";
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

  if (op_size > 0) {
    memset(msgbuf, 'A', op_size - 1);
    msgbuf[op_size - 1] = 0;
  } else {
    msgbuf[0] = 0;
  }
  
  /* send and recv messages on the endpoints */
  /* regular endpoints */
  send (ep1,ep2,msgbuf,status,MCAPI_SUCCESS);
  recv (ep2,status,MCAPI_SUCCESS,recvbuf);
  if (strcmp(msgbuf,recvbuf)) {
    printf ("   Test FAILED\n");
    rc++;
  } else {
    printf("   Test PASSED\n");
  }
  
  mcapi_finalize(&status);
  
  return rc;
}
