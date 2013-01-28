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
/* Test: msg2
   Description: Tests simple non-blocking msgsend and msgrecv calls between endpoints
   on two nodes.  This is a pthreaded test where one thread/node is the sender and the 
   other is the receiver.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mca.h>
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <pthread.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define NUM_THREADS 2
#define BUFF_SIZE 64
#define PORT_NUM 1


mcapi_endpoint_t endpoints[NUM_THREADS];

#define WRONG wrong(__LINE__);
void wrong(unsigned line)
{
  mca_status_t status;
  mcapi_finalize(&status);
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}


typedef struct { 
  int  thread_id;
  mcapi_endpoint_t endpoint;
  int recv_node;
} thread_data;

thread_data thread_data_array[NUM_THREADS];


void send (mcapi_endpoint_t send, mcapi_endpoint_t recv,char* msg,mca_status_t status,int exp_status) {
  int size = strlen(msg);
  int priority = 1;
  printf("//send...\n");
  mcapi_msg_send(send,recv,msg,size,priority,&status);
  if (status != exp_status) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("//endpoint=%i has sent: [%s]\n",(int)send,msg);
  }
}

void recv (mcapi_endpoint_t recv,mca_status_t status,int exp_status) {
  size_t recv_size;
  char buffer[BUFF_SIZE];

  printf("//recv...\n");
  mcapi_msg_recv(recv,buffer,BUFF_SIZE,&recv_size,&status);
  if (status != exp_status) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG}
  if (status == MCAPI_SUCCESS) {
    printf("//endpoint=%i has received: [%s]\n",(int)recv,buffer);
  }
}

void* run_thread (void *t) {

  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;
  char msg[BUFF_SIZE];
  mcapi_endpoint_t send_endpt,recv_endpt;
  int tid;
  
  thread_data *my_data = (thread_data *)t;
  tid = my_data->thread_id;

  mca_node_t node_num = NODE+tid;


  /* create a node */
  printf("//1) run_thread: tid=%i\n",tid);
 
  mcapi_initialize(DOMAIN,node_num,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
  
  if (!(tid % 2)) {
    /* odd threads receive */
    recv_endpt = mcapi_endpoint_create(PORT_NUM,&status);
    if (status != MCAPI_SUCCESS) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
    printf("//tid %i receiving...\n",tid);
    recv (recv_endpt,status,MCAPI_SUCCESS);
  } else {
    /* even threads send */
    send_endpt = mcapi_endpoint_create(PORT_NUM,&status);
    if (status != MCAPI_SUCCESS) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
    printf("//tid %i sending...\n",tid);
    /* get the receive endpoint */
    recv_endpt = mcapi_endpoint_get (DOMAIN,node_num-1,PORT_NUM,MCA_INFINITE,&status);
    if (status != MCAPI_SUCCESS) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

    /* try to delete an endpoint that we don't own */
    mcapi_endpoint_delete (recv_endpt,&status);
    if (status != MCAPI_ERR_ENDP_NOTOWNER ) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

    sprintf (msg,"//Guten tag MCAPI from ep:%i to ep:%i tid:%i",(int)send_endpt,(int)recv_endpt,tid);
    send (send_endpt,recv_endpt,msg,status,MCAPI_SUCCESS);
  
  }
  
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) { printf("//status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

  return NULL;
}

int
 main () {
  pthread_t threads[NUM_THREADS];
  int rc, t;

  mcapi_set_debug_level(1);

  for(t=0; t<NUM_THREADS; t++){
    thread_data_array[t].thread_id = t;
  }
  /* run all the threads */
  for(t=0; t<NUM_THREADS; t++){
    rc = pthread_create(&threads[t], NULL, run_thread, (void *)&thread_data_array[t]);
    if (rc){
      fprintf(stderr,"//ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t],NULL);
  }

  if (rc == 0) {
    printf("  //TEST passed\n");
  }
  return rc;
}
