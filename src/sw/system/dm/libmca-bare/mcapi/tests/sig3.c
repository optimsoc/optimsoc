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
// An MPMT signal handling test where each thread does not have it's own signal handler.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <mca.h>
#include <mcapi.h> 

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <pthread.h>


#define SEND_PORT 1
#define RECV_PORT 0

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#define BUFF_SIZE 256


static mca_domain_t the_domain = DOMAIN;
static mca_node_t the_main_node = NODE;
static mca_node_t the_second_node = NODE+1;
static mca_node_t the_other_main_node = NODE+2;
static mca_node_t the_other_second_node = NODE+3;
static mcapi_endpoint_t thread2_recv_endpt;
static mcapi_endpoint_t thread2_send_endpt;
static mcapi_endpoint_t main_recv_endpt;
static mcapi_endpoint_t main_send_endpt;
static int priority = 1;

void finalize(int failed) {
  mca_status_t mcapi_status;

  mcapi_finalize(&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: (pid %d) failed to finalize MCAPI: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
  }

  if (failed) { 
    printf ("\n******************** FAILED pid%d tid%lu\n",getpid(),(unsigned long)pthread_self());
  }
}

void* second_thread(void* data)
{
  mca_status_t mcapi_status;
  mcapi_info_t mcapi_version;
  mcapi_param_t parms;
  int rc = 0;
  char msg2 [BUFF_SIZE];
  size_t recv_size;
  /* do some MCAPI work */

  printf("In thread 2, pid=%d\n",getpid());

  mcapi_initialize(the_domain, 
                   the_second_node, 
                   NULL,&parms,
                   &mcapi_version, 
                   &mcapi_status);

  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: thread 2 (pid %d) failed to initialize MCAPI: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; 
     finalize(rc); 
     return NULL;
  }

  printf("Thread 2 INITIALIZE DONE\n");

  thread2_recv_endpt = 
    mcapi_endpoint_create(RECV_PORT, &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: thread 2 (pid %d) failed to create recv endpoint: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; finalize(rc); return NULL;
  }

  printf("Thread 2 CREATE RECV ENDPT DONE\n");

  thread2_send_endpt = 
    mcapi_endpoint_create(SEND_PORT, &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: thread 2 (pid %d) failed to create send endpoint: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; finalize(rc); return NULL;
  }

  printf("Thread 2 CREATE SEND ENDPT DONE\n");

  /* get the remote endpoint */
  main_recv_endpt = mcapi_endpoint_get (the_domain,the_main_node,RECV_PORT,MCA_INFINITE
                                        ,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    printf("//status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    finalize(1);
    return NULL;
  }

  /* send a msg */
  sprintf (msg2,"//THREAD 2: DONE from ep:%i to ep:%i ",(int)thread2_send_endpt,(int)main_recv_endpt);
  mcapi_msg_send(thread2_send_endpt,main_recv_endpt,msg2,strlen(msg2),priority,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) { 
    fprintf(stderr, "Thread 2 failed to send message\n");
    finalize(1);
    return NULL;
  }
  
  /* recv a msg */
  mcapi_msg_recv(thread2_recv_endpt,msg2,BUFF_SIZE,&recv_size,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) { 
    fprintf(stderr, "Thread 2 failed to recv message\n");
    finalize(1); 
    return NULL;
  }   else {
    printf("THREAD 2 received: %s\n",msg2);
  }
  
  raise(SIGINT);
   
  return NULL;
}

int do_main_thread(void)
{
  mca_status_t mcapi_status;
  mcapi_info_t mcapi_version;
  mcapi_param_t parms;
  int rc = 0;
  char msg1 [BUFF_SIZE];
  size_t recv_size;

  printf("In main thread, pid=%d\n",getpid());

  mcapi_initialize(the_domain, 
                   the_main_node, 
                   NULL,&parms,
                   &mcapi_version, 
                   &mcapi_status);

  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: thread 2 (pid %d) failed to initialize MCAPI: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; finalize(rc); return rc;
  }

  printf("Main Thread INITIALIZE DONE\n");

  main_recv_endpt = 
    mcapi_endpoint_create(RECV_PORT, &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: main thread (pid %d) failed to create recv endpoint: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; finalize(rc); return rc;
  }

  printf("Main Thread CREATE RECV ENDPT DONE\n");

  main_send_endpt = 
    mcapi_endpoint_create(SEND_PORT, &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: main thread (pid %d) failed to create send endpoint: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    rc++; finalize(rc); return rc;
  }

  printf("Main Thread CREATE SEND ENDPT DONE\n");

   /* get the remote endpoint */
  thread2_recv_endpt = mcapi_endpoint_get (the_domain,the_second_node,RECV_PORT,MCA_INFINITE
                                        ,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    printf("//status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    finalize(1);
    return 1;
  }

  /* recv a msg */
  mcapi_msg_recv(main_recv_endpt,msg1,BUFF_SIZE,&recv_size,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) { 
    fprintf(stderr, "Main thread failed to recv message\n");
    finalize(1); 
    return 1;
  }   else {
    printf("Main thread received: %s\n",msg1);
  }

  /* send a msg */
  sprintf (msg1,"//Main Thread: DONE from ep:%i to ep:%i ",(int)main_send_endpt,(int)thread2_recv_endpt);
  mcapi_msg_send(main_send_endpt,thread2_recv_endpt,msg1,strlen(msg1),priority,&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) { 
    fprintf(stderr, "Main thread failed to send message\n");
    finalize(1);
    return 1;
  }
  
 
  finalize(rc); 
  return rc;
}

int do_fork(void)
{
  pid_t pid;
  pthread_t thread2;
  int thread_rc;
  int rc;
  int null_data;

  /* child will. register the task */
  /* parent will invoke and wait on it */

  if ((pid = fork()) < 0) {
    /* error forking */
    fprintf(stderr,"ERROR: error performing fork() call!\n");
    exit(3);
  } else if (pid == 0) {

    /* in child */
        
    printf("In child, pid = %d\n",getpid());

    the_main_node = 2;
    the_second_node = 3;
    the_other_main_node = 0;
    the_other_second_node = 1;

    pthread_create(&thread2,NULL,second_thread,&thread_rc);
    rc = do_main_thread();
    pthread_join(thread2,(void*)&null_data);
    printf("CHILD DONE (proc_rc=%d thread_rc=%d)\n",rc,thread_rc);
    rc |= thread_rc;
  } else {

    /* in parent */

    printf("In parent, pid=%d\n",getpid());

    the_main_node = 0;
    the_second_node = 1;
    the_other_main_node = 2;
    the_other_second_node = 3;

    pthread_create(&thread2,NULL,second_thread,&thread_rc);
    rc = do_main_thread();
    pthread_join(thread2,(void*)&null_data);
    printf("PARENT DONE (proc_rc=%d thread_rc=%d)\n",rc,thread_rc);
    rc |= thread_rc;
  }
  
  return rc;
}

int main(int argc, char** argv)
{

  mcapi_set_debug_level(1);
  if (do_fork() != 0) {
    fprintf(stderr,"ERROR: failed during do_fork!\n");
    exit(1);
  }
  exit(0);
}
