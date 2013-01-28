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
/* Test: msg17
   Description: Tests that sends complete immediately and their requests are freed. 
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


#define PORT_NUM 100

#define BUFF_SIZE 64

#define NUM_RECEIVERS (MCA_MAX_REQUESTS/MCAPI_MAX_QUEUE_ELEMENTS)

typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} r_entry;

r_entry requests[NUM_RECEIVERS*MCAPI_MAX_QUEUE_ELEMENTS];


#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  mcapi_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mcapi_finalize(&status);
  exit(1);
}

void send (mcapi_endpoint_t send, mcapi_endpoint_t recv,char* msg,int exp_status,int i) {
  int size = strlen(msg);
  int priority = 1;
  
  mcapi_msg_send_i(send,recv,msg,size,priority,&requests[i].request,&requests[i].status);
  if (requests[i].status == MCAPI_SUCCESS) { 
    mcapi_test(&requests[i].request,&requests[i].size,&requests[i].status);
    if (requests[i].status != exp_status) { 
      printf("found: %s\n",mcapi_display_status(requests[i].status,status_buff,sizeof(status_buff)));
      printf("exp: %s\n",mcapi_display_status(exp_status,status_buff,sizeof(status_buff)));
      WRONG
        }
    
    printf("endpoint=%i has sent: [%s]\n",(int)send,msg);
  }
}

void recv (mcapi_endpoint_t recv,int exp_status) {
 
  char buffer[BUFF_SIZE];
  mca_status_t status;
  mca_request_t request;
  mcapi_msg_recv_i(recv,buffer,BUFF_SIZE,&request,&status);
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("endpoint=%i has received: [%s]\n",(int)recv,buffer);
  }
}

int main () {
  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_endpoint_t ep1,ep2;
  int j;
  int num_requests = 0;
  mcapi_set_debug_level(1);
 
   /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  /* create endpoints */
  ep1 = mcapi_endpoint_create(500,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(600,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  // these should all complete
  for (j = 0; j < MCAPI_MAX_QUEUE_ELEMENTS; j++) {
    num_requests++;
    send (ep1,ep2,"Hello MCAPI",MCAPI_SUCCESS,j);
  }

  // these should all hit mem_limit
  for (j = 0; j < MCA_MAX_REQUESTS; j++) {
    num_requests++;
    send (ep1,ep2,"Hello MCAPI",MCAPI_ERR_MEM_LIMIT,j);
  }

  printf ("nr=%d\n",num_requests);
  // now try to do one receive, it should be able to get a request entry and complete
  recv(ep2,MCAPI_SUCCESS);


  
  // make sure the test hit the case where our nb sends exceeded the total number of requests 
  if (num_requests < MCA_MAX_REQUESTS) { 
    return 1; 
  }
  mcapi_finalize(&status);
  
  printf("   Test PASSED\n");
  return 0;
}
