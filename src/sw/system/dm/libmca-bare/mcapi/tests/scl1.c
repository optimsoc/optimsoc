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
/* Test: scl1
   Description: Tests scl_channel_send and scl_channel_recv calls on single node.  
   Note, that for scalar channels we only have blocking versions of send/recv.
   This test tests send/recv to several endpoints on the same node.  It tests
   all error conditions.  
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
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
#define PORT_NUM3 300
#define PORT_NUM4 400


#define BUFF_SIZE 64

#define NUM_SIZES 4 

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

mcapi_boolean_t send (mcapi_sclchan_send_hndl_t send_handle, mcapi_endpoint_t recv,unsigned long long data,uint32_t size,mca_status_t status,int exp_status) {
  mcapi_boolean_t rc = MCAPI_FALSE;
  switch (size) {
  case (8): mcapi_sclchan_send_uint8(send_handle,data,&status); break;
  case (16): mcapi_sclchan_send_uint16(send_handle,data,&status); break;
  case (32): mcapi_sclchan_send_uint32(send_handle,data,&status); break;
  case (64): mcapi_sclchan_send_uint64(send_handle,data,&status); break;
  default: fprintf (stderr,"ERROR: bad data size in call to send\n");
  };
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has sent %i byte(s): [%llu]\n",(int)send_handle,(int)size/8,data);
  }
  if (status == exp_status) {
    rc = MCAPI_TRUE;
  }
  return rc;
}

mcapi_boolean_t recv (mcapi_sclchan_recv_hndl_t recv_handle,uint32_t size,mca_status_t status,uint64_t exp_status,unsigned long long exp_data) {
  unsigned long long data = 0;
  mcapi_boolean_t rc = MCAPI_FALSE;
  uint64_t size_mask = 0; 
  switch (size) {
  case (8): size_mask = 0xff;data=mcapi_sclchan_recv_uint8(recv_handle,&status); break;
  case (16): size_mask = 0xffff;data=mcapi_sclchan_recv_uint16(recv_handle,&status); break;
  case (32): size_mask = 0xffffffff;data=mcapi_sclchan_recv_uint32(recv_handle,&status); break;
  case (64): size_mask = 0xffffffffffffffffULL;data=mcapi_sclchan_recv_uint64(recv_handle,&status); break;
  default: fprintf (stderr,"ERROR: bad data size in call to send\n");
  };
 
  exp_data = exp_data & size_mask;
   
  if (status == exp_status) {
    rc = MCAPI_TRUE;
  }
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"endpoint=%i has received %i byte(s): [%llu]\n",(int)recv_handle,(int)size/8,data);
    if (data != exp_data) { 
       fprintf(stderr, "expected %lld, received %lld\n",exp_data,data); 
       rc = MCAPI_FALSE; 
     }
  }
 
  return rc;
}

int main () {
  mca_status_t status;
  mcapi_request_t request;
  mcapi_endpoint_t ep1,ep2,ep3,ep4,ep5,ep6,ep7,ep8,ep9,ep10;
 
  /* cases:
     1: both named endpoints (1,2)
     2: both anonymous endpoints (3,4)
     3: anonymous sender, named receiver (5,6)
     4: anonymous receiver, named sender (7,8)
  */
  mcapi_sclchan_send_hndl_t s1,s2,s3,s4,s5; /* s1 = ep1->ep2, s2 = ep3->ep4, s3 = ep5->ep6, s4 = ep7->ep8 */
  mcapi_sclchan_recv_hndl_t r1,r2,r3,r4,r5; /* r1 = ep1->ep2, r2 = ep3->ep4, r3 = ep5->ep6, r4 = ep7->ep8 */
  mcapi_uint_t avail;
  int s;
  int i;
  int sizes[NUM_SIZES] = {8,16,32,64};
  size_t size;
mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_boolean_t rc = MCAPI_FALSE;
  mcapi_set_debug_level(6);

  /* initialize these to invalid handle values */
  s5 = 0xdeadbeef;
  r5 = 0xdeadbee0;
  ep9 = 0xdeadbabe;
  ep9 = 0xdeadcafe;

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
  rc = send (s1,ep2,1,32,status,MCAPI_ERR_CHAN_INVALID); 
  if (!rc) {WRONG}
  rc = recv (r1,32,status,MCAPI_ERR_CHAN_INVALID,0);
  if (!rc) {WRONG}

  /*************************** connect the channels *********************/
  /* error: not an endpoint */
do {
    mcapi_sclchan_send_open_i(&s1 /*send_handle*/,ep9, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }

  /* error: not endpoints */
  ep10 = 0xbabecafe;
do {
    mcapi_sclchan_connect_i(ep9,ep10,&request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }

do {
    mcapi_sclchan_connect_i(ep1,ep2,&request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }

  /* error: already connected */
do {
    mcapi_sclchan_connect_i(ep1,ep2,&request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_CHAN_CONNECTED) { WRONG }
  
do {
    mcapi_sclchan_connect_i(ep3,ep4, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_sclchan_connect_i(ep5,ep6, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
  
do {
    mcapi_sclchan_connect_i(ep7,ep8, &request, &status);                                                                                                                                                                          
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG } 

  /*************************** open the channels *********************/
  /* error: invalid endpoint */
do {
    mcapi_sclchan_recv_open_i(&r1 /*recv_handle*/,ep9, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }
  /* error: wrong send/receive direction */
do {
    mcapi_sclchan_send_open_i(&r1 /*recv_handle*/,ep2, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_CHAN_DIRECTION) { WRONG }
do {
    mcapi_sclchan_recv_open_i(&s1 /*send_handle*/,ep1, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_ERR_CHAN_DIRECTION) { WRONG }

  r1 = 0xdeadbeef;
  s1 = 0xdeadbee0;

  /*  error:send/recv on unopen channel */
  rc = send (s1,ep3,2,32,status,MCAPI_ERR_CHAN_INVALID);
  if (!rc) {WRONG}
  rc = recv (r1,32,status,MCAPI_ERR_CHAN_INVALID,0);
  if (!rc) {WRONG}
 
  /* now open the channels */
do {
    mcapi_sclchan_recv_open_i(&r1 /*recv_handle*/,ep2, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_recv_open_i(&r2 /*recv_handle*/,ep4, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_recv_open_i(&r3 /*recv_handle*/,ep6, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_recv_open_i(&r4 /*recv_handle*/,ep8, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_send_open_i(&s1 /*send_handle*/,ep1, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_send_open_i(&s2 /*send_handle*/,ep3, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_send_open_i(&s3 /*send_handle*/,ep5, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }
do {
    mcapi_sclchan_send_open_i(&s4 /*send_handle*/,ep7, &request, &status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  if (status != MCAPI_SUCCESS) { WRONG }

  /* test send/recv of different sizes */
  rc = send (s1,ep2,3,8,status,MCAPI_SUCCESS);
  if (!rc) {WRONG}
  rc = recv (r1,16,status,MCAPI_ERR_SCL_SIZE,3);
  if (!rc) {WRONG}
  rc = send (s1,ep2,3,8,status,MCAPI_SUCCESS);
  if (!rc) {WRONG}
  rc = recv (r1,32,status,MCAPI_ERR_SCL_SIZE,3);
  if (!rc) {WRONG}
  rc = send (s1,ep2,3,8,status,MCAPI_SUCCESS);
  if (!rc) {WRONG}
  rc = recv (r1,64,status,MCAPI_ERR_SCL_SIZE,3);
  if (!rc) {WRONG}
  rc = send (s1,ep2,3,16,status,MCAPI_SUCCESS);
  if (!rc) {WRONG}
  rc = recv (r1,8,status,MCAPI_ERR_SCL_SIZE,3);
  if (!rc) {WRONG}

  for (s = 0; s < NUM_SIZES; s++) {
    size = sizes[s];
    fprintf(stderr,"***************** size=%i\n",(int)size);
    /* send and recv messages on the channels */
    /* regular endpoints */
    rc = send (s1,ep2,3,size,status,MCAPI_SUCCESS);
    if (!rc) {WRONG}
    rc = send (s2,ep4,4,size,status,MCAPI_SUCCESS);
    if (!rc) {WRONG}
    mcapi_display_state(&r1);
    rc = recv (r1,size,status,MCAPI_SUCCESS,3);  /* PROBLEM IS HERE, receives 0 */
    if (!rc) {WRONG}
    rc = recv (r2,size,status,MCAPI_SUCCESS,4);
    if (!rc) {WRONG}
    rc = send (s3,ep6,5,size,status,MCAPI_SUCCESS);
    if (!rc) {WRONG}
    rc = recv (r3,size,status,MCAPI_SUCCESS,5);
    if (!rc) {WRONG}
    rc = send (s4,ep8,6,size,status,MCAPI_SUCCESS);
    if (!rc) {WRONG}
    rc = recv (r4,size,status,MCAPI_SUCCESS,6);
    if (!rc) {WRONG}
    
    /* error: a non-existent channel */
    rc = recv (r5,size,status,MCAPI_ERR_CHAN_INVALID,0);
    if (!rc) {WRONG}
    rc = send (s5,ep4,7,size,status,MCAPI_ERR_CHAN_INVALID);
    if (!rc) {WRONG}
    
    /*  error: queue full
        test the case where the endpoints receive queue is full and thus there
        are no more buffers available */
    fprintf(stderr,"testing full queue...\n");
    for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
      rc = send (s1,ep2,10+i,size,status,MCAPI_SUCCESS);
      if (!rc) {WRONG}
    }
    
    rc = send (s1,ep2,8,size,status,MCAPI_ERR_MEM_LIMIT);
    if (!rc) {WRONG}
    rc = send (s1,ep2,16,size,status,MCAPI_ERR_MEM_LIMIT);
    if (!rc) {WRONG}
    rc = send (s1,ep2,32,size,status,MCAPI_ERR_MEM_LIMIT);
    if (!rc) {WRONG}
    rc = send (s1,ep2,64,size,status,MCAPI_ERR_MEM_LIMIT);
    if (!rc) {WRONG}
    
    for (i = 0; i < MCAPI_MAX_QUEUE_ELEMENTS; i++) {
      rc = recv (ep2,size,status,MCAPI_SUCCESS,10+i);
      if (!rc) {WRONG}
    }
    
    
    /* check if data is available */
    rc = send (s1,ep2,9,size,status,MCAPI_SUCCESS);
    if (!rc) {WRONG}
    avail = mcapi_sclchan_available(r1, &status);
    if (status != MCAPI_SUCCESS) { WRONG }    
    if (avail != 1) { WRONG }
    avail = mcapi_sclchan_available(r2, &status);
    if (status != MCAPI_SUCCESS) { WRONG }    
    if (avail != 0) { WRONG }
    /* error: invalid handle */
    mcapi_sclchan_available(r5, &status);
    if (status != MCAPI_ERR_CHAN_INVALID) { WRONG }   
    rc = recv (r1,size,status,MCAPI_SUCCESS,9);
    if (!rc) {WRONG}
  }
  
  /* close the channels */
do {
    mcapi_sclchan_recv_close_i(r1,&request,&status);     
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_recv_close_i(r2,&request,&status);    
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_recv_close_i(r3,&request,&status);     
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_recv_close_i(r4,&request,&status);    
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }                                                                                                                                                                    

do {
    mcapi_sclchan_send_close_i(s1,&request,&status); 
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_send_close_i(s2,&request,&status);  
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_send_close_i(s3,&request,&status); 
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }

do {
    mcapi_sclchan_send_close_i(s4,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
   if (status != MCAPI_SUCCESS) { WRONG }
  
  mcapi_finalize(&status);

  printf("\n\n   Test PASSED\n");
  return 0;
}
