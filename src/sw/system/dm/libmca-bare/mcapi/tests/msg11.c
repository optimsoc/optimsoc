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
/* Test: msg11
   Description: This test was written because the pthreaded tests were finding bugs 
   that we couldn't reproduce.  This test uses rand in the standard library and seeds
   the generator with gettimeofday.  The order of the calls (send or recv) is chosen
   randomly as well as the number of packets sent or received each time.  For this 
   test, the get_random function returns a number between 1 and 10.  It can take 
   the seed as a command line argument when trying to reproduce a given test.  This
   test can overflow the queue.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <string.h>
#include <sys/time.h> /* for seeding with gettimeofday */
#include <stdlib.h> /* for rand,srand */

#ifndef BUFF_SIZE
#define BUFF_SIZE 64
#endif

/* want possiblity of overflowing the queue */
#define NUM_MSGS (BUFF_SIZE*3) 


#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  mcapi_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mcapi_finalize(&status);
  exit(1);
}

typedef struct {
  mcapi_boolean_t completed;
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} send_entry;

typedef struct {
  mcapi_boolean_t completed;
  mcapi_request_t request;
  mca_status_t status;
  char buffer[BUFF_SIZE];
  size_t size;
} recv_entry;

send_entry send_requests[NUM_MSGS];
recv_entry recv_requests[NUM_MSGS];
int i_s = 0;
int i_r = 0;

  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;

mcapi_boolean_t sender();
mcapi_boolean_t receiver();
int check_results();

unsigned get_random() {
  /* for now, restrict the random number to between 1:10 */
  unsigned x = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
  return x;
}



mcapi_boolean_t collect_requests (mcapi_boolean_t done,mcapi_status_t reason) {
  int i;
  static int timeout = 0;
  timeout++;
  mca_status_t status;
  int n;
  int r = 0;
  char status_buff[128];
  if (TIMEOUT && (timeout >= TIMEOUT)) {
    fprintf(stderr,"FAIL: deadlock: send or recv unable to proceed\n   Test FAILED\n");
    // mcapi_display_state(&r1);
    mcapi_finalize(&status);
    exit(1);
  }


  /* gather up all the receives */
  n =  i_r;
  for (i = 0; i < n; i++) {
    if (recv_requests[i].completed) { continue;}
    
    if (mcapi_test(&recv_requests[i].request,&recv_requests[i].size,&recv_requests[i].status)) {
      if (recv_requests[i].status == MCAPI_SUCCESS) {
        recv_requests[i].completed = MCAPI_TRUE;
        r++;
      } else {
        fprintf(stderr,"\nERROR: collect_results, mcapi_trans_test: %s\n",
                mcapi_display_status(recv_requests[i].status,status_buff,sizeof(status_buff)));
        WRONG
      }
    }
  }
  printf("collect_requests collected %i requests\n",r);
  if ((reason == MCAPI_ERR_REQUEST_LIMIT) && (r == 0)) {
    mcapi_finalize(&status);
    printf("  DEADLOCK\n");
    exit(1);
  }
  return MCAPI_TRUE;
}



int main(int argc, char** argv) {
  mcapi_boolean_t recv_done = MCAPI_FALSE;
  mcapi_boolean_t send_done = MCAPI_FALSE;

  mca_status_t status;

  int num_sends,num_recvs,order,i;
  struct timeval tv;
  unsigned int seed;

  int rc = 0;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_set_debug_level(6);

  memset(&send_requests,0,sizeof(send_requests));
  memset(&recv_requests,0,sizeof(recv_requests));
 
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
    fprintf(stderr,"\nERROR: Failed to initialize (0): %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    rc++;
    mcapi_finalize(&status); 
    return rc; 
  }
  
  /* create the endpoints */
  send_endpt = mcapi_endpoint_create(1,&status);
  recv_endpt =  mcapi_endpoint_create(2,&status);

 
  /* randomly issue sends and receives */
  printf("sending and receiving num_msgs=%d\n",NUM_MSGS);
  while (!(recv_done && send_done)) {
    if (send_done) { order = 1; }
   else if (recv_done) { order = 0; }
   else { order = get_random(); }
    if (order % 2) {
      /* do some receives */
      num_recvs = get_random();
      printf("issuing %i receives...\n",num_recvs); 
      for (i = 0; i < num_recvs; i++) {
       recv_done = receiver();
      }
    } else {
      /* do some sends */
      num_sends = get_random();
      printf("issuing %i sends...\n",num_sends); 
      for (i = 0; i < num_sends; i++) {
        send_done = sender();
      }
    } 
  }
  
  rc =check_results();
  
  
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"\nERROR: Failed to finalize: %s\n",mcapi_display_status(status,status_buff,sizeof(status_buff)));  
    rc++;
  }
  
  
  if (rc == 0) {
    printf("   Test PASSED\n");
  } else {
    printf("   Test FAILED\n");
  }
 return rc;
}

mcapi_boolean_t sender()
{
  int done = 0;
  int sent = 0;
  char status_buff[BUFF_SIZE];
  if (i_s >= NUM_MSGS) {  printf("sender done hit NUM_MSGS\n"); return MCAPI_TRUE; }

  sprintf(send_requests[i_s].buffer,"Sending: %d",i_s);
  send_requests[i_s].size=strlen(send_requests[i_s].buffer);
 
  // for non-blocking, we have to check for both MEM_LIMIT and REQUEST_LIMIT errors
  //  and retry as needed
  while (!done) {
    while (!sent) {
      mcapi_msg_send_i(send_endpt,
                       recv_endpt,
                       send_requests[i_s].buffer,
                       send_requests[i_s].size,
                       1,
                       &send_requests[i_s].request,
                       &send_requests[i_s].status);
      if (send_requests[i_s].status == MCAPI_ERR_REQUEST_LIMIT) {
        printf("send out of request handles\n");
        collect_requests(MCAPI_FALSE,send_requests[i_s].status);
      } else if (send_requests[i_s].status != MCAPI_SUCCESS) {
        WRONG
      } else {
        sent = 1;
      }
    }
    send_requests[i_s].completed = mcapi_test(&send_requests[i_s].request,&send_requests[i_s].size,&send_requests[i_s].status);
    if (send_requests[i_s].status == MCAPI_ERR_MEM_LIMIT) {
      printf("send out of entries in the recv queue\n");
      collect_requests(MCAPI_FALSE,send_requests[i_s].status);
      //sent = 0; 
      return MCAPI_FALSE;
    } else if (send_requests[i_s].status != MCAPI_SUCCESS) {
      printf("ERROR: %s\n",
             mcapi_display_status(send_requests[i_s].status,status_buff,sizeof(status_buff)));
      WRONG
        } else {
      done = 1;
      i_s++;
    }
  }
      
  return MCAPI_FALSE;
}


mcapi_boolean_t receiver()
{
  int done = 0;
  int recd = 0;
  char status_buff[BUFF_SIZE];

  if (i_r >= NUM_MSGS) { printf(" receiver done hit NUM_MSGS\n"); return MCAPI_TRUE; }

  // for non-blocking, we have to check for both MEM_LIMIT and REQUEST_LIMIT errors
  //  and retry as needed
  while (!done) {
    while (!recd) {
      mcapi_msg_recv_i(recv_endpt,
                       (void **)((void*)&recv_requests[i_r].buffer),
                       sizeof(recv_requests[i_r].buffer),
                       &recv_requests[i_r].request,
                       &recv_requests[i_r].status);
 
      if (recv_requests[i_r].status == MCAPI_ERR_REQUEST_LIMIT) {
        printf("recv out of request handles\n");
        collect_requests(MCAPI_FALSE,recv_requests[i_r].status);
      } else if (recv_requests[i_r].status != MCAPI_SUCCESS) {
        WRONG
      } else {
        recd = 1;
      }
    }
    recv_requests[i_r].completed = mcapi_test(&recv_requests[i_r].request,&recv_requests[i_r].size,&recv_requests[i_r].status);
    if (recv_requests[i_r].status == MCAPI_ERR_MEM_LIMIT) {
      printf("recv out of entries in the recv queue\n");
      collect_requests(MCAPI_FALSE,recv_requests[i_r].status);
      //recd = 0;
      return MCAPI_FALSE;
    } else if (recv_requests[i_r].status != MCAPI_SUCCESS) {
      printf("ERROR: %s\n",
             mcapi_display_status(recv_requests[i_r].status,status_buff,sizeof(status_buff)));
      WRONG
    } else {
      done = 1;
      i_r++;
    }
  }
   
  return MCAPI_FALSE;
}

int check_results () {
  int i;
  int rc = 0;
  char send_status_buff[128];
  char recv_status_buff[128];

  /* collect any outstanding receives */
  collect_requests(MCAPI_TRUE,0);

  /* test that the data sent matches the data received */
  for (i = 0; i < NUM_MSGS; i++) {
    /* make sure the status is success */
    if ((send_requests[i].status != MCAPI_SUCCESS) ||
        (recv_requests[i].status != MCAPI_SUCCESS) ) {
      fprintf(stderr,"FAIL: send status=%s received status=%s (expected MCAPI_SUCCESS for both)\n",
              mcapi_display_status(send_requests[i].status,send_status_buff,sizeof(status_buff)),
              mcapi_display_status(recv_requests[i].status,recv_status_buff,sizeof(status_buff)));
      rc++;
    }
    /* make sure the sizes match and they are non-zero */
    if ((send_requests[i].size != recv_requests[i].size) ||
        (send_requests[i].size == 0) || (recv_requests[i].size == 0)){
      fprintf(stderr,"FAIL: send size=%i,recv size=%i\n",(int)send_requests[i].size,(int)recv_requests[i].size);
      rc++;
      
    } else {
      /* make sure the data matches */
      if (strcmp(send_requests[i].buffer,recv_requests[i].buffer)) {
        fprintf(stderr,"FAIL: send data[%s] != recv data[%s]\n",send_requests[i].buffer,recv_requests[i].buffer);
        rc++;
      }
      printf("(%d) sent: [%s], recvd: [%s]\n",i,send_requests[i].buffer,recv_requests[i].buffer);
    }
  }
  return rc;
}

