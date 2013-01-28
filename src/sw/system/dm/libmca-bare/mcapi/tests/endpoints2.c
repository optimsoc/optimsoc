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
/* Test: endpoints2
   Description: This test is very similar to endpoints1 except that it tests the 
   non-blocking endpoint_get_i for valid endpoints as well as invalid. 

*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mcapi_impl_spec.h>
#include <stdio.h>
#include <stdlib.h> /* for exit */
#include <mca_config.h>


#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#ifndef PORT
#define PORT 15
#endif

#define FAIL 1
#define SUCCESS 0

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

int main () {
  mca_status_t status;
mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_request_t request;
  size_t size;
  int domain_num,node_num,i;
  mcapi_endpoint_t ep[MCAPI_MAX_ENDPOINTS];
  mcapi_endpoint_t ep1,ep2,ep3;

  mcapi_set_debug_level(6);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  /* get my node_num */
  node_num = mcapi_node_id_get (&status);
  if (node_num != NODE) { WRONG }
 
  /* get my domain num */
  domain_num = mcapi_domain_id_get (&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  if (domain_num != DOMAIN)  { WRONG }
 
  /* create an endpoint */
  ep1 = mcapi_endpoint_create(PORT,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  /* create endpoint */
  ep2 =mcapi_endpoint_create(MCAPI_PORT_ANY,&status);
  if (status != MCAPI_SUCCESS) { WRONG}
  
  /* lookup a valid endpoint */
do {
    mcapi_endpoint_get_i (DOMAIN,NODE, PORT,&ep3,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (ep3 != ep1) { WRONG}

  /* delete the valid endpoints we've created so far */
  mcapi_endpoint_delete(ep1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  mcapi_endpoint_delete(ep2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  /* delete an invalid endpoint */
  /* ep3 == ep1 and we already deleted ep1, so expect an error */
  mcapi_endpoint_delete(ep3,&status);
  if (status != MCAPI_ERR_ENDP_INVALID) { WRONG }


  /* try to get an endpoint that never existed (and no endpoints currently exist) */
do {
    mcapi_endpoint_get_i (DOMAIN,NODE, 10,&ep1,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  mcapi_wait(&request,&size,0xff,&status);
  if (status == MCAPI_SUCCESS) { WRONG }
   
  
  /* create the max number of endpoints (since there are no endpoints now, there should be room) */
  for (i = 0; i < MCAPI_MAX_ENDPOINTS; i++) {
    ep[i] = mcapi_endpoint_create(i,&status);
    if (status != MCAPI_SUCCESS) { WRONG }
  }

  /* try to create one more endpoint, should fail */
  ep1 = mcapi_endpoint_create(PORT,&status);
  if (status == MCAPI_SUCCESS) { WRONG }

  /* lookup a valid endpoint  */
do {
    mcapi_endpoint_get_i (DOMAIN,NODE, 10,&ep1,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  while (!mcapi_test(&request,&size,&status)) {}
  if (status != MCAPI_SUCCESS) { WRONG }
  if (ep1 != ep[10]) { WRONG }
  
  /* lookup an invalid endpoint (never existed)*/
do {
    mcapi_endpoint_get_i (DOMAIN,NODE, 100,&ep1,&request,&status);
//retry if all request handles are in-use
} while (status == MCAPI_ERR_REQUEST_LIMIT);
  mcapi_wait(&request,&size,0xff,&status); 
  if (status == MCAPI_SUCCESS) { WRONG }

  /* set an attribute */
  
  /* get an attribute */
   int available_buffers = -1;
   mcapi_endpoint_get_attribute(
        ep1,
        MCAPI_ENDP_ATTR_NUM_RECV_BUFFERS,
        &available_buffers,
        sizeof(available_buffers),
        &status);
  if (status != MCAPI_SUCCESS) { WRONG } 
  if (available_buffers != (MCAPI_MAX_QUEUE_ELEMENTS+1))  { WRONG } 

  mcapi_finalize(&status);
  printf("   Test PASSED\n");
  return SUCCESS;
}
