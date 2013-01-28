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
/* Test: msg1
   Description: Tests simple blocking msgsend and msgrecv calls to several endpoints
   on a single node 
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mcapi_impl_spec.h>
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <mca_config.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define PORT_NUM1 100
#define PORT_NUM2 200

#define BUFF_SIZE 64

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  mcapi_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mcapi_finalize(&status);
  exit(1);
}

void send (mcapi_endpoint_t send, mcapi_endpoint_t recv,char* msg,mca_status_t status,int exp_status) {
  int size = strlen(msg);
  int priority = 1;

  if (exp_status == MCAPI_ERR_MSG_LIMIT) {
    size = MCAPI_MAX_MSG_SIZE+1;
  }

  if (exp_status == MCAPI_ERR_PRIORITY) {
    priority = MCAPI_MAX_PRIORITY + 1;
  }

  mcapi_msg_send(send,recv,msg,size,priority,&status);
  if (status != exp_status) { printf("expected %s, found %s",mcapi_display_status(exp_status,status_buff,sizeof(status_buff)),mcapi_display_status(status,status_buff,sizeof(status_buff)));WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("endpoint=%i has sent: [%s]\n",(int)send,msg);
  }
}

void recv (mcapi_endpoint_t recv,mca_status_t status,int exp_status) {
  size_t recv_size;
  char buffer[BUFF_SIZE];
  mcapi_msg_recv(recv,buffer,BUFF_SIZE,&recv_size,&status);
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("endpoint=%i has received: [%s]\n",(int)recv,buffer);
  }
}

int main () {
  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_endpoint_t ep1,ep2,ep3,ep4,ep5;
  int i;

  mcapi_set_debug_level(1);

  ep5 = 0xdeadbeef; /* invalid endpoint */
  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
    
  /* create endpoints */
  ep1 = mcapi_endpoint_create(PORT_NUM1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(PORT_NUM2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  /* create anonymous endpoints */
  ep3 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG}

  ep4 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG}

  /* send and recv messages on the endpoints */
  /* regular endpoints */
  send (ep1,ep2,"Hello MCAPI",status,MCAPI_SUCCESS);
  recv (ep2,status,MCAPI_SUCCESS);

  /* both anonymous endpoints */
  send (ep3,ep4,"Guten Tag MCAPI",status,MCAPI_SUCCESS);
  recv (ep4,status,MCAPI_SUCCESS);

  /* check for truncation */
  send (ep3,ep4,"This message is too long and will be truncated XXXXXXXXXXXXXXXXXXXXX.",status,MCAPI_ERR_MSG_LIMIT);

  /* error: invalid priority */
  send (ep3,ep4,"ERROR",status,MCAPI_ERR_PRIORITY);

  /* one anonymous, one regular endpoint */
  send (ep1,ep3,"Hola MCAPI",status,MCAPI_SUCCESS);
  recv (ep3,status,MCAPI_SUCCESS);
  send (ep4,ep2,"Bonjour MCAPI",status,MCAPI_SUCCESS);
  if (mcapi_msg_available(ep2,&status) != 1) { WRONG } /* check msg available works */
  if (status != MCAPI_SUCCESS) { WRONG }
  recv (ep2,status,MCAPI_SUCCESS);
  if (mcapi_msg_available(ep2,&status) != 0) { WRONG } /* check msg available works */
  if (status != MCAPI_SUCCESS) { WRONG }

  if (mcapi_msg_available(ep5,&status) != 0) { WRONG } /* check msg available works */
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }

 
  /* a non-existent endpoint */
  send (ep4,ep5,"Ciao MCAPI",status,MCAPI_ERR_ENDP_INVALID);
  recv (ep5,status,MCAPI_ERR_ENDP_INVALID);

  send (ep1,ep3,"Hola MCAPI",status,MCAPI_ERR_MSG_LIMIT);

  /* test the case where the endpoints receive queue is full and thus there
     are no more buffers available */
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    send (ep1,ep2,"filling queue...",status,MCAPI_SUCCESS);
  }
  send (ep1,ep2,"queue should be full...",status,MCAPI_ERR_MEM_LIMIT);
  
  mcapi_finalize(&status);
  printf("   Test PASSED\n");
  return 0;
}
