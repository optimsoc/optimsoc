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
/*
   Description: A simple work pool program that performs matrix mutliplication
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 3
#define PORT_NUM 1
#define BUFF_SIZE 10
#define NUM_ASSIGNMENTS 10

//opCodes
#define OP_REQUEST 0
#define OP_ASSIGN 1
#define OP_REPORT 2

#define DOMAIN 1


mcapi_endpoint_t endpoints[NUM_THREADS];

#define ERROR error(__LINE__);
void error(unsigned line) {
  mcapi_status_t status;
  mcapi_finalize(&status);
  exit(1);
}

void check_success(mcapi_status_t status, int line_no) {
  if (status != MCAPI_SUCCESS) { 
    printf("ERROR: status=%s line_no=%d",mcapi_display_status(status,status_buff,sizeof(status_buff)),line_no); 
  }
}




typedef struct {
  int  thread_id;
  mcapi_endpoint_t endpoint;
  int recv_node;
  int rc;
} thread_data;

struct task{
  int index;
  struct task* next;
};

thread_data thread_data_array[NUM_THREADS];
volatile mcapi_boolean_t done = MCAPI_FALSE;

/**
* The method that is run by each thread
**/
void* run_thread (void *t) {
  // Setup
  mcapi_status_t status;
  mcapi_info_t version;
  mcapi_param_t params;
  int tid;
  int i,j;
  thread_data *my_data = (thread_data *)t;
  tid = my_data->thread_id;
  mcapi_endpoint_t myRecvEndpoints[NUM_THREADS+1];
  mcapi_endpoint_t foreignRecvEndpoints[NUM_THREADS+1];
  mcapi_endpoint_t sendEndpoints[NUM_THREADS+1];
  
  // Setup an output file
  FILE *file;
  char name[20];
  sprintf (name,"out%d.txt", tid); 
  file = fopen(name,"w");
  fprintf(file, "out%d\n", tid);
  fclose(file);
  file = fopen(name,"a");
  
  int A[NUM_ASSIGNMENTS][BUFF_SIZE];
  int B[NUM_ASSIGNMENTS][BUFF_SIZE];
  int C[NUM_ASSIGNMENTS][BUFF_SIZE];
  for(i=0; i<NUM_ASSIGNMENTS; i++){
    for(j=0; j<NUM_ASSIGNMENTS; j++){
      A[i][j] = i+j;
      B[i][j] = i+j;
      C[i][j] = 0;
    }
  }
  for(i=0; i<=NUM_THREADS; i++){
    myRecvEndpoints[i] = -1;
    foreignRecvEndpoints[i] = -1;
    sendEndpoints[i] = -1;
  }

  //assignments holds the id of the node responible for that task.
  // so the node respondible for the 8th assignment can be found with
  // assignments[8]
  int portion = NUM_ASSIGNMENTS / NUM_THREADS;
  int assignments[NUM_ASSIGNMENTS];
  int currNode = 1;
  for(i=0; i<NUM_ASSIGNMENTS; i++){
    if(i >= currNode * portion && currNode < NUM_THREADS){
      currNode++;
    }
    assignments[i] = currNode;
  }
  
  
  for(i=0; i<NUM_ASSIGNMENTS; i++){
    fprintf(file,"assignments[%d] = node: %d\n",i,assignments[i]);
  }


  // Create a node
  mcapi_initialize(DOMAIN,tid,NULL,&params,&version,&status);
  check_success(status,__LINE__);
    


  // Create sufficient endpoints to connect all nodes to all others
  for(i=1; i<=NUM_THREADS; i++){
    if (i==tid)
      continue;
    myRecvEndpoints[i] = mcapi_endpoint_create (tid*10+i,&status);
    check_success(status,__LINE__);
  }
  for(i=1; i<=NUM_THREADS; i++){
    if (i==tid)
      continue;
    foreignRecvEndpoints[i] = mcapi_endpoint_get(DOMAIN, i,i*10+tid, MCA_INFINITE, &status);
    check_success(status,__LINE__);
    sendEndpoints[i] = mcapi_endpoint_create ((i+tid)*10,&status);
    check_success(status,__LINE__);
  }
  

  //Do your work
  int assignment;
  for(assignment = 0; assignment < NUM_ASSIGNMENTS; assignment++){
    if(assignments[assignment] == tid){ //if this is my responsibility
      for(i=0; i<NUM_ASSIGNMENTS; i++){
        C[assignment][i] += A[i][assignment] * B[assignment][i];
      }
      assignments[assignment] = -1; //so in the future we know we already did it	
    }
  }
  

  fprintf(file,"*****************   TRIAL 1 *************************\n"); 
  //Give the root node the complete results
  if(tid == 1){
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      int nodeID = assignments[i]; //I'm choosing to read from node 2 first then node 3
      if(nodeID != -1){ //If I wasn't the one to do the work
        //int buffer[NUM_ASSIGNMENTS+2];
        int buffer[100];
        
        size_t recv_size;
        fprintf(file,"Receiving from node %x at %x\n", (unsigned)nodeID, (unsigned)myRecvEndpoints[nodeID]);
        mcapi_msg_recv(myRecvEndpoints[nodeID],buffer,(NUM_ASSIGNMENTS+2)*sizeof(int),&recv_size,&status);
        check_success(status,__LINE__);
        
        char msg[100];
        sprintf (msg,"Got msg %d %d %d %d %d %d %d %d %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
        fprintf(file,"%s",msg);fclose(file);file = fopen(name,"a");
        
        int assignmentIndex = buffer[1];
        for(j=0; j<NUM_ASSIGNMENTS; j++){
          C[assignmentIndex][j] = buffer[j+2]; //+2 so we skip the assignment index and OPCODE
        }
      }
    }
    fprintf(file,"Finished Receiving\n");
    fclose(file);
    file = fopen(name,"a");
    // done is a global but since only TID=1 writes it, it's safe
    done = MCAPI_TRUE;
  }
  else{
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      if(assignments[i] == -1){ //If I was the one to do the work
        int msg[NUM_ASSIGNMENTS+2];
        //int *msg = malloc(sizeof(int)*(NUM_ASSIGNMENTS+2));
        for(j=0; j<NUM_ASSIGNMENTS; j++){
          msg[j+2] = C[i][j];//j+2 so we leave index 0 and 1 for metadata
        }
        msg[0] = OP_REPORT;
        msg[1] = i;
        mcapi_msg_send(sendEndpoints[1],foreignRecvEndpoints[1],msg,(NUM_ASSIGNMENTS+2)*sizeof(int),1,&status);
        char buffer[100];
        sprintf (buffer,"Sent msg %d %d %d %d %d %d %d %d %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]);
        fprintf(file,"%s",buffer);
      }
    }
    fprintf(file,"Finished Sending\n");
  }
  

  //Print all three matrixes
  if(tid == 1){
    fprintf(file,"\nA\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", A[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    fprintf(file,"\nB\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", B[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    fprintf(file,"\nC\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", C[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        if (C[i][j] != A[i][j] * B[i][j]) {
          fprintf(file,"FAIL: expected C[%d][%d] == A[%d][%d] * B[%d][%d] == %d, found %d\n",i,j,i,j,i,j,A[i][j]*B[i][j],C[i][j]);
          printf("FAIL: expected C[%d][%d] == A[%d][%d] * B[%d][%d] == %d, found %d\n",i,j,i,j,i,j,A[i][j]*B[i][j],C[i][j]);
          my_data->rc++;
        }
      }
    }
  }

  
  /************************************************************************
   * Now we will send the exact same messages again, but without the wait *
   ************************************************************************/
  
  fprintf(file,"*****************   TRIAL 2 *************************\n"); 
  //Give the root node the complete results
  if(tid == 1){
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      int nodeID = assignments[i]; //I'm choosing to read from node 2 first then node 3
      if(nodeID != -1){ //If I wasn't the one to do the work
  			//int buffer[NUM_ASSIGNMENTS+2];
        int buffer[100];
        size_t recv_size;
        fprintf(file,"Receiving from node %x at endpoint_handle=0x%x\n", nodeID, myRecvEndpoints[nodeID]);
        mcapi_msg_recv(myRecvEndpoints[nodeID],buffer,(NUM_ASSIGNMENTS+2)*sizeof(int),&recv_size,&status);
        check_success(status,__LINE__);
        
        if (recv_size != (NUM_ASSIGNMENTS+2)*sizeof(int)) {
          fprintf(file,"ERROR2: line=%u status=%s size was:%lu, expected:%lu\n", __LINE__, mcapi_display_status(status,status_buff,sizeof(status_buff)), (long unsigned)recv_size, (long unsigned)(NUM_ASSIGNMENTS+2)*sizeof(int));
          fclose(file);
          ERROR
            }
        char msg[100];
        sprintf (msg,"Got msg %d %d %d %d %d %d %d %d %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
        fprintf(file,"%s",msg);fclose(file);file = fopen(name,"a");
  	
        int assignmentIndex = buffer[1];
        for(j=0; j<NUM_ASSIGNMENTS; j++){
          C[assignmentIndex][j] = buffer[j+2]; //+2 so we skip the assignment index and OPCODE
        }
      }
    }
    fprintf(file,"Finished Receiving\n");
    fclose(file);
    file = fopen(name,"a");
  }
  else{
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      if(assignments[i] == -1){ //If I was the one to do the work
        int msg[NUM_ASSIGNMENTS+2];
        //int *msg = malloc(sizeof(int)*(NUM_ASSIGNMENTS+2));
        for(j=0; j<NUM_ASSIGNMENTS; j++){
          msg[j+2] = C[i][j];//j+2 so we leave index 0 and 1 for metadata
        }
        msg[0] = OP_REPORT;
        msg[1] = i;
        mcapi_msg_send(sendEndpoints[1],foreignRecvEndpoints[1],msg,(NUM_ASSIGNMENTS+2)*sizeof(int),1,&status);
        char buffer[100];
        sprintf (buffer,"tid%d sent from ep=%x to ep=%x msg: %d %d %d %d %d %d %d %d %d %d %d %d\n", tid,sendEndpoints[1],foreignRecvEndpoints[1],msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]);
        fprintf(file,buffer);
      }
    }
    fprintf(file,"Finished Sending\n");
  }
  
  //Print all three matrixes
  if(tid == 1){
    fprintf(file,"\nA\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", A[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    fprintf(file,"\nB\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", B[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    fprintf(file,"\nC\n");
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        char msg[10];sprintf(msg,"%d\t", C[i][j]);fprintf(file,"%s",msg);
      }
      fprintf(file,"\n");
    }
    
    
    for(i=0; i<NUM_ASSIGNMENTS; i++){
      for(j=0; j<NUM_ASSIGNMENTS; j++){
        if (C[i][j] != A[i][j] * B[i][j]) {
          fprintf(file,"FAIL: expected C[%d][%d] == A[%d][%d] * B[%d][%d] == %d, found %d\n",i,j,i,j,i,j,A[i][j]*B[i][j],C[i][j]);
          my_data->rc++;  
        }
      }
    }
  }
  
  //Finalize and report
  fprintf(file,"Closing\n");
  fclose(file);
  
  while (!done) {}
 

  mcapi_finalize(&status);
  check_success(status,__LINE__);
  

  printf("Thread=%d done, file=%s written rc=%d\n",tid,name,my_data->rc);
  return NULL;
}

/**
 * Create all threads, have them do their jobs, join all of them, report outcome
**/
int main () {
  // Setup
  pthread_t threads[NUM_THREADS];
  int rc = 0;
  int t;
  //mcapi_set_debug_level(6);
  
  // Create and run all the threads
  for(t=0; t<NUM_THREADS; t++){
    thread_data_array[t].thread_id = t+1;
    thread_data_array[t].rc = 0;
  }
  for(t=0; t<NUM_THREADS; t++){
    rc = pthread_create(&threads[t], NULL, run_thread, (void *)&thread_data_array[t]);
    if (rc){
      fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  // Wait for all threads and report result
  for (t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t],NULL);
  }
  
  printf("checking return codes...\n");
  for(t=0; t<NUM_THREADS; t++){
    rc += thread_data_array[t].rc;
    printf("  Thread[%d] returned rc=%d\n",t,thread_data_array[t].rc);
  }

  if (rc == 0) {
    printf("\nPASSED\n");
  } else {
    printf("\nFAILED\n");
  }
  return rc;
}
