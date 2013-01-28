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
/* Test: pkt2
   Description: Tests simple in-order non-blocking pkt_channel_send and 
   pkt_channel_recv calls between endpoints on a single node.  Tests all
   error conditions.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mcapi_impl_spec.h>

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <assert.h>
#include <mca_config.h>

#define PORT_NUM1 100
#define PORT_NUM2 200
#define PORT_NUM3 300
#define PORT_NUM4 400

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

int buffs_used = 0;


#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

void* buffers [MCAPI_MAX_BUFFERS+1];
int num_elements = 0;

void send (mcapi_pktchan_send_hndl_t send_handle, mcapi_endpoint_t recv,char* msg,mca_status_t status,int exp_status) {
  mcapi_request_t request;
  int size = strlen(msg);
  size_t sent_size;

   if (exp_status == MCAPI_ERR_PKT_LIMIT) {
    size = MCAPI_MAX_PKT_SIZE+1;
  }
   else { assert (size <= MCAPI_MAX_PKT_SIZE); }

do {
    mcapi_pktchan_send_i(send_handle,msg,size,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&sent_size,&status)) {} /* these requests should complete right away */
  if (status != exp_status) {printf(" expected status=%s, found status=%s\n",mcapi_display_status(exp_status,status_buff,sizeof(status_buff)),mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG}
  if (size != sent_size) {
    printf("Error: expected_size=%i,received_size=%i\n",(int)size,(int)sent_size);
  }
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has sent: [%s]\n",(int)send_handle,msg);
    buffs_used++;
  }
}

void recv (int free_buffer,mcapi_pktchan_recv_hndl_t recv_handle,mca_status_t status,int exp_status,char* exp_data) {
  size_t recv_size;
  mcapi_request_t request;
  char *buffer;
do {
    mcapi_pktchan_recv_i(recv_handle,(void **)((void*)&buffer),&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&recv_size,&status)) {} /* these requests should complete right away */
  if (status != exp_status) { WRONG}
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has received %i bytes: [%s]\n",(int)recv_handle,(int)recv_size,buffer);
    if (strcmp(exp_data,buffer)) {
      printf("expected [%s], but received: [%s]\n",exp_data,buffer);
      WRONG
    }
    if (free_buffer) {
      mcapi_pktchan_release((void *)buffer,&status);
      if (status != MCAPI_SUCCESS) { WRONG} 
      buffs_used--;
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
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_endpoint_t ep1,ep2,ep3,ep4,ep5,ep6,ep7,ep8,ep9;
  /* cases:
     1: both named endpoints (1,2)
     2: both anonymous endpoints (3,4)
     3: anonymous sender, named receiver (5,6)
     4: anonymous receiver, named sender (7,8)
  */
  mcapi_pktchan_send_hndl_t s1,s2,s3,s4,s5; /* s1 = ep1->ep2, s2 = ep3->ep4, s3 = ep5->ep6, s4 = ep7->ep8 */
  mcapi_pktchan_recv_hndl_t r1,r2,r3,r4,r5; /* r1 = ep1->ep2, r2 = ep3->ep4, r3 = ep5->ep6, r4 = ep7->ep8 */
  mcapi_uint_t avail;
  int i;

 
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    buffers[i] = NULL;
  }

  /* initialize these to invalid handle values */
  s5 = 0xdeadbeef;
  r5 = 0xdeadbee0;
  ep9 = 0xdeadbabe;

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


  /* try send and receive on unconnected channel */
  send (s1,ep2,"blah",status,MCAPI_ERR_CHAN_INVALID);
  recv (1,r1,status,MCAPI_ERR_CHAN_INVALID,"");
  
  /*************************** connect the channels *********************/
do {
    mcapi_pktchan_connect_i(ep1,ep2,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }

  /* error: already connected */
do {
    mcapi_pktchan_connect_i(ep2,ep4, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_ERR_CHAN_CONNECTED) { WRONG }

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
  /* error: invalid endpoint */
do {
    mcapi_pktchan_recv_open_i(&r1 /*recv_handle*/,ep9, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }
  
  /* error: wrong send/receive direction */
do {
    mcapi_pktchan_send_open_i(&s1 /*recv_handle*/,ep2, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_ERR_CHAN_DIRECTION) { WRONG }

do {
    mcapi_pktchan_recv_open_i(&r1 /*send_handle*/,ep1, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_ERR_CHAN_DIRECTION) { WRONG }

  /*  error:send/recv on unopen channel */
  send (s5,ep3,"Hola MCAPI",status,MCAPI_ERR_CHAN_INVALID);
  recv (1,r5,status,MCAPI_ERR_CHAN_INVALID,"");
 
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


  /* send and recv messages on the channels */
  /* regular endpoints */
  send (s1,ep2,"Hello MCAPI",status,MCAPI_SUCCESS);
  send (s2,ep4,"Hola MCAPI",status,MCAPI_SUCCESS);
  recv (1,r1,status,MCAPI_SUCCESS,"Hello MCAPI");
  recv (1,r2,status,MCAPI_SUCCESS,"Hola MCAPI");
  send (s3,ep6,"Bonjour MCAPI",status,MCAPI_SUCCESS);
  recv (1,r3,status,MCAPI_SUCCESS,"Bonjour MCAPI");
  send (s4,ep8,"Ciao MCAPI",status,MCAPI_SUCCESS);
  recv (1,r4,status,MCAPI_SUCCESS,"Ciao MCAPI");

  /* error: too large of a packet */
  send (s4,ep8,"Ciao MCAPI",status,MCAPI_ERR_PKT_LIMIT);

  /* error: a non-existent channel */
  recv (1,r5,status,MCAPI_ERR_CHAN_INVALID,"");
  send (s5,ep4,"blah",status,MCAPI_ERR_CHAN_INVALID);

  /*  error: queue full
      test the case where the endpoints receive queue is full */ 
    for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
      send (s1,ep2,"filling queue...",status,MCAPI_SUCCESS);
    }
    /* endpoints receive queue is full */
    send (s1,ep2,"queue full...",status,MCAPI_ERR_MEM_LIMIT);
    for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
      recv (0,ep2,status,MCAPI_SUCCESS,"filling queue...");
    }

  while (buffs_used < MCAPI_MAX_BUFFERS) {
    send (s1,ep2,"depleting...",status,MCAPI_SUCCESS);
    recv (0,ep2,status,MCAPI_SUCCESS,"depleting...");
  }
  printf ("buffs_used=%i\n",buffs_used);

  /* error: endpoints receive queue is not full, but there are no more buffers available */
  send (s1,ep2,"none avail...",status,MCAPI_ERR_MEM_LIMIT);

  /* free one buffer */
  for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
    if (buffers[i] != NULL) {
      mcapi_pktchan_release((void *)buffers[i],&status);
      buffers[i] = NULL;
      break;
    }
  }
  send (s1,ep2,"filling queue..",status,MCAPI_SUCCESS);
  recv (1,ep2,status,MCAPI_SUCCESS,"filling queue..");

  /* check if packets are available */
  send (s1,ep2,"pkt avail",status,MCAPI_SUCCESS);
  avail = mcapi_pktchan_available(r1, &status);
  if (status != MCAPI_SUCCESS) { WRONG }    
  if (!avail) { WRONG }
  avail = mcapi_pktchan_available(r2, &status);
  if (status != MCAPI_SUCCESS) { WRONG }    
  if (avail) { WRONG }
  /* error: invalid handle */
  mcapi_pktchan_available(r5, &status);
  if (status != MCAPI_ERR_CHAN_INVALID) { WRONG } 

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
