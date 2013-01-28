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
// An MPMT messaging test.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mca.h>
#include <mcapi.h> 

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <signal.h>


#define SEND_PORT 1
#define RECV_PORT 0

#ifndef NODE
#define NODE 0
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define BUFF_SIZE 512

void finalize () {
  mcapi_status_t mcapi_status;

  mcapi_finalize(&mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    fprintf(stderr,"ERROR: parent (pid %d) failed to finalize MCAPI: %s\n",
            getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
  } 
}

int do_fork(void)
{
  pid_t pid;
  char msg1 [BUFF_SIZE];
  char msg2 [BUFF_SIZE];
  mcapi_node_t the_main_node;
  mcapi_node_t the_other_main_node;
  mcapi_param_t parms;
  mcapi_info_t mcapi_version;
  mcapi_status_t mcapi_status;
  mcapi_endpoint_t main_send_endpt;
  mcapi_endpoint_t main_recv_endpt;
  mcapi_endpoint_t recv_endpt;
  mcapi_endpoint_t send_endpt;
  size_t recv_size;
  mcapi_priority_t priority = 1;

  mcapi_set_debug_level(5);
  if ((pid = fork()) < 0) {
    /* error forking */
    fprintf(stderr,"ERROR: error performing fork() call!\n");
    exit(3);
  } else if (pid == 0) {

    /* in child */
        
    printf("In child, pid = %d\n",getpid());

    the_main_node = NODE+1;
    the_other_main_node = NODE;

    mcapi_initialize(DOMAIN, 
                     the_main_node, 
                     NULL,&parms,
                     &mcapi_version, 
                     &mcapi_status);
    
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR:  (pid %d) failed to initialize MCAPI: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Child Process INITIALIZE DONE\n");
    
    main_recv_endpt = 
      mcapi_endpoint_create(RECV_PORT, &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR: child process (pid %d) failed to create recv endpoint: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Child Process CREATE RECV ENDPT DONE\n");
    
    main_send_endpt = 
      mcapi_endpoint_create(SEND_PORT, &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR: child process (pid %d) failed to create send endpoint: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Child Process CREATE SEND ENDPT DONE\n");
    
    /* get th remote endpoints */
    recv_endpt = mcapi_endpoint_get (DOMAIN,the_other_main_node,RECV_PORT,MCA_INFINITE,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      printf("//mcapi_status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff))); 
      finalize(); 
      return 1;
    }
    
    send_endpt = mcapi_endpoint_get (DOMAIN,the_other_main_node,SEND_PORT,MCA_INFINITE,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      printf("//status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff))); 
      finalize(); 
      return 1;
    }

    /* send a msg */
    sprintf (msg1,"//CHILD: Guten tag MCAPI from ep:%i to ep:%i ",(int)send_endpt,(int)recv_endpt);
    mcapi_msg_send(main_send_endpt,recv_endpt,msg1,strlen(msg1),priority,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      fprintf(stderr, "Child failed to send message\n");
      finalize();
      return 1;
    }
    printf("Child Process BLOCKING SEND DONE\n");

    /* recv a msg */
    mcapi_msg_recv(main_recv_endpt,msg1,BUFF_SIZE,&recv_size,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      fprintf(stderr, "Child failed to recv message\n");
      finalize(); return 1;
    }   else {
      printf("CHILD received: %s\n",msg1);
    }
    printf("Child Process BLOCKING RECV DONE\n");

    /* finalize */
    finalize();
    
    printf("CHILD DONE\n");
    
  } else {
    
    /* in parent */

    printf("In parent, pid=%d\n",getpid());

    the_main_node = NODE;
    the_other_main_node = NODE+1;

    mcapi_initialize(DOMAIN, 
                     the_main_node, 
                     NULL,&parms,
                     &mcapi_version, 
                     &mcapi_status);
    
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR: parent process (pid %d) failed to initialize MCAPI: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Parent Process INITIALIZE DONE\n");
    
    main_recv_endpt = 
      mcapi_endpoint_create(RECV_PORT, &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR: parent process (pid %d) failed to create recv endpoint: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Parent Process CREATE RECV ENDPT DONE\n");
    
    main_send_endpt = 
      mcapi_endpoint_create(SEND_PORT, &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      fprintf(stderr,"ERROR: parent process (pid %d) failed to create send endpoint: %s\n",
              getpid(),mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      finalize(); return 1;
    }
    
    printf("Parent Process CREATE SEND ENDPT DONE\n");

    /* get th remote endpoints */
    recv_endpt = mcapi_endpoint_get (DOMAIN,the_other_main_node,RECV_PORT,MCA_INFINITE,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      printf("//mcapi_status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff))); 
      finalize(); 
      return 1;
    }

    send_endpt = mcapi_endpoint_get (DOMAIN,the_other_main_node,SEND_PORT,MCA_INFINITE,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      printf("//status=%s",mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff))); 
      finalize(); 
      return 1;
    }

    /* send a msg */
    sprintf (msg2,"//PARENT: Guten tag MCAPI from ep:%i to ep:%i ",(int)send_endpt,(int)recv_endpt);
    mcapi_msg_send(main_send_endpt,recv_endpt,msg2,strlen(msg2),priority,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      fprintf(stderr, "Parent failed to send message\n");
      finalize();
      return 1;
    }
    printf("Parent Process BLOCKING SEND DONE\n");

    /* recv a msg */
    mcapi_msg_recv(main_recv_endpt,msg2,BUFF_SIZE,&recv_size,&mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) { 
      fprintf(stderr, "Parent failed to recv message\n");
      finalize(); return 1;
    }   else {
      printf("PARENT received: %s\n",msg2);
    }
    printf("Parent Process BLOCKING RECV DONE\n");


    /* finalize */
    finalize();

    printf("PARENT DONE\n");
            
  }
  
  return 0;
}

int main(int argc, char** argv)
{

  if (do_fork() != 0) {
    fprintf(stderr,"ERROR: failed during do_fork!\n");
    exit(1);
  } 
  
  printf("\nPASSED\n");

  exit(0);
}
