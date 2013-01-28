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
/* Test: msg7
   Description: This test was written because the pthreaded tests were finding bugs 
   that we couldn't reproduce.  This test uses rand in the standard library and seeds
   the generator with gettimeofday.  The order of the calls (send or recv) is chosen
   randomly as well as the number of messages sent or received each time.  For this 
   test, the get_random function returns a number between 1 and 10.  It can take 
   the seed as a command line argument when trying to reproduce a given test.  This
   test does not test overflowing the queue.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <string.h>
#include <sys/time.h> /* for seeding with gettimeofday */
#include <stdlib.h> /* for rand,srand */
#include <mca_config.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#ifndef BUFF_SIZE
#define BUFF_SIZE 128 
#endif

/* avoid overflowing the queue */
#define NUM_MSGS (MCAPI_MAX_QUEUE_ELEMENTS-1) 

typedef struct {
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} request_entry;

request_entry send_requests[NUM_MSGS];
request_entry recv_requests[NUM_MSGS];

const int fail = 1;
const int ok = 0;

mcapi_boolean_t sender(mcapi_endpoint_t send_endpt,mcapi_endpoint_t recv_endpt);
mcapi_boolean_t receiver(mcapi_endpoint_t recv_endpt);
mcapi_boolean_t check_results();

unsigned get_random() {
  /* for now, restrict the random number to between 1:10 */
  unsigned x = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
  return x;
}

int main(int argc, char** argv)
{
  mcapi_boolean_t recv_done = MCAPI_FALSE;
  mcapi_boolean_t send_done = MCAPI_FALSE;
  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;
  mca_status_t status;
  int num_sends,num_recvs,order,i;
  struct timeval tv;
  unsigned int seed;
mcapi_param_t parms;
  mcapi_info_t version;

  mcapi_set_debug_level(4);

  /* initialize the random number generator with a seed */
  if (argc == 2) {
    seed = atoi(argv[1]);
  } else {
    gettimeofday(&tv,0);
    seed = tv.tv_sec + tv.tv_usec;
  }
  printf("Using seed:[%u]\n",seed);
  srand(seed);

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to initialize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    return fail;
  }
  
  /* create the endpoints */
  send_endpt = mcapi_endpoint_create(1,&status);
  recv_endpt =  mcapi_endpoint_create(2,&status);

  /* randomly issue sends and receives */
  while (!(recv_done && send_done)) {
    order = get_random();
    if (order % 2) {
      /* if we chose an odd number, do some receives */
      num_recvs = get_random();
      if (!recv_done) { printf("issuing %i receives...\n",num_recvs); }
      for (i = 0; i < num_recvs; i++) {
        recv_done = receiver(recv_endpt);
      }
    } else {
      /* if we chose an even number, do some sends */
      num_sends = get_random();
      if (!send_done) {printf("issuing %i sends...\n",num_sends); }
      for (i = 0; i < num_sends; i++) {
        send_done = sender(send_endpt,recv_endpt);
      }
    }
  }
  
  if (! check_results()) {
    fprintf(stderr,"\nFAILED - check results\n");
    mcapi_finalize(&status);
    return fail;
  }

  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    return fail;
  }
  
  printf("   Test PASSED\n");
  return ok;
}

mcapi_boolean_t sender(mcapi_endpoint_t send_endpt,mcapi_endpoint_t recv_endpt)
{
  static int i = 0;
  mcapi_priority_t priority = 1;
  if (i >= NUM_MSGS) { return MCAPI_TRUE; }
  sprintf(send_requests[i].buffer,"Sending: %d",i);
  send_requests[i].size=strlen(send_requests[i].buffer);
  do {
    mcapi_msg_send_i(send_endpt,
                     recv_endpt,
                     send_requests[i].buffer,
                     send_requests[i].size,
                     priority,
                     &send_requests[i].request,
                     &send_requests[i].status);
  } while (send_requests[i].status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
  return MCAPI_FALSE;
}


mcapi_boolean_t receiver(mcapi_endpoint_t recv_endpt)
{  

  static int i = 0;
  if (i >= NUM_MSGS) { return MCAPI_TRUE; }
  do {
    mcapi_msg_recv_i(recv_endpt,
                     recv_requests[i].buffer,
                     sizeof(recv_requests[i].buffer),
                     &recv_requests[i].request,
                     &recv_requests[i].status );
  } while (recv_requests[i].status == MCAPI_ERR_REQUEST_LIMIT);
  i++;
  return MCAPI_FALSE;
}

mcapi_boolean_t check_results () 
{
  size_t size;
  int i;

  /* gather up all the sends */
  for (i = 0; i < NUM_MSGS; i++) {
    mcapi_test(&send_requests[i].request,&size,&send_requests[i].status);
    //if (send_requests[i].status == MCAPI_INCOMPLETE) {
    //  fprintf(stderr,"FAIL: found incomplete send request %i\n",i);
    //  return MCAPI_FALSE; /* sends should all be complete */
    //}  
    if (send_requests[i].size !=  size) {
      fprintf(stderr,"FAIL: send: size returned by test[%i] != sent size[%i]\n",
              (int)size,(int)send_requests[i].size); 
      return MCAPI_FALSE;
    }
  }
  
  /* gather up all the receives */
  for (i = 0; i < NUM_MSGS; i++) {
    recv_requests[i].size = sizeof (recv_requests[i].buffer);
    mcapi_test(&recv_requests[i].request,&recv_requests[i].size,&recv_requests[i].status);
    //if (recv_requests[i].status == MCAPI_INCOMPLETE) {
    //  fprintf(stderr,"FAIL: found incomplete receive request %i\n",i);
    //  return MCAPI_FALSE; /* since the sends have completed, the receives should now be complete */
    //}
    if (recv_requests[i].size !=  send_requests[i].size) {
      fprintf(stderr,"FAIL: recv: size returned by test[%i] != sent size[%i]\n",
              (int)size,(int)send_requests[i].size); 
      return MCAPI_FALSE;
    }
  }

  /* test that the data sent matches the data received */
  for (i = 0; i < NUM_MSGS; i++) {
    if ((send_requests[i].status != MCAPI_SUCCESS) ||
        (recv_requests[i].status != MCAPI_SUCCESS) ) {
      fprintf(stderr,"FAIL: send status=%s received status=%s (expected MCAPI_SUCCESS for both)\n",
              mcapi_display_status(send_requests[i].status,status_buff,sizeof(status_buff)),
              mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff)));
      return MCAPI_FALSE;
    }
    if ((send_requests[i].size != recv_requests[i].size) ||
        (send_requests[i].size == 0) || (recv_requests[i].size == 0)){
      fprintf(stderr,"FAIL: send size=%i,recv size=%i\n",(int)send_requests[i].size,(int)recv_requests[i].size);
      return MCAPI_FALSE;
    }
    if (strcmp(send_requests[i].buffer,recv_requests[i].buffer)) {
      fprintf(stderr,"FAIL: send data[%s] != recv data[%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
      return MCAPI_FALSE;
    }
    printf("success: sent: [%s], recvd: [%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
  }
  return MCAPI_TRUE;
}



