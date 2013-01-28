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
/* Basic test for scatter/gather remote memory functionality */

#include <mrapi.h>


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


/* this test is specifically written for two threads */
#define NUM_THREADS 2

#define WRONG(x) wrong(__LINE__,x);
#define RMEM_KEY 0xdeadbabe
#define LOCK_LIMIT 1
#define BUFF_SIZE 1024

typedef struct {
  mrapi_rmem_hndl_t rmem; /* mrapi rmem handle */
  mca_node_t tid;
  mca_node_t tid_other;
} thread_data;

void wrong(unsigned line,thread_data t)
{
  mca_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mrapi_rmem_detach(t.rmem,&status);
  mrapi_rmem_delete(t.rmem,&status);
  mrapi_finalize(&status);
  exit(1);
}


void *run_thread(void* tdata) {
  mca_status_t status;
  int offset;
  thread_data* t = (thread_data*)tdata;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  size_t msg_size;
  char buf[BUFF_SIZE]; // the remote memory
  char msg [BUFF_SIZE];  // what we will write from
  char read_buf[BUFF_SIZE]; // what we will read into
  char expected_msg [BUFF_SIZE];  // what we expect to read
  char status_buff[BUFF_SIZE];
 
  memset(buf,0,BUFF_SIZE);
  memset(msg,0,BUFF_SIZE);
  memset(read_buf,0,BUFF_SIZE);
  memset(expected_msg,0,BUFF_SIZE);

  printf("run_thread tid=%d\n",t->tid);
  /* initialize mrapi */
  mrapi_initialize(DOMAIN,NODE+t->tid,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
  
  /* create or get the remote memory and attach to it */
  t->rmem = mrapi_rmem_create(RMEM_KEY,&buf,MRAPI_RMEM_DUMMY,NULL /*attrs*/,sizeof(buf) /*size*/,&status);
  if (status == MRAPI_ERR_RMEM_EXISTS) {
    //printf("rmem exists, try getting it from the other node");
    t->rmem = mrapi_rmem_get(RMEM_KEY,MRAPI_RMEM_DUMMY,&status);
  }
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  mrapi_rmem_attach(t->rmem,MRAPI_RMEM_DUMMY,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
  
  // write the message
  sprintf(msg,"sAcBaCtDtEeFrG HgIaJtKhLeMrN OtPeQsRtS!T!U!V!W!X!Y!Zabcdefghijklmnopqrstuvwxyz%d",t->tid);

  sprintf(expected_msg,"scatter gather test!!!!");
  msg_size = strlen(msg);

  offset = t->tid * BUFF_SIZE/NUM_THREADS + 4;
  mrapi_rmem_write(
                   t->rmem,
                   offset,
                   msg,
                   0,
                   1,
                   msg_size,
                   1,
                   1,
                   &status
                   );
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  // write the valid bit
  offset = t->tid * BUFF_SIZE/NUM_THREADS;
  mrapi_rmem_write(
                   t->rmem,
                   offset,
                   "V",
                   0,
                   strlen("V"),
                   1,
                   1*strlen("V"),
                   1*strlen("V"),
                   &status
                   );
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

  // spin on the other thread's valid bit
  offset = t->tid_other * BUFF_SIZE/NUM_THREADS;
  while (strcmp(read_buf,"V")) {
    mrapi_rmem_read(
                    t->rmem,
                    offset,
                    read_buf,
                    0,
                    strlen("V"),
                    1,
                    1*strlen("V"),
                    1*strlen("V"),
                    &status
                    );
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) } 
  }

  // now read the other thread's message
  offset = t->tid_other * BUFF_SIZE/NUM_THREADS + 4;
  mrapi_rmem_read(
                  t->rmem,
                  offset,
                  read_buf,
                  0,
                  1,
                  23,
                  2,
                  1,
                  &status
                  );
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) } 
  printf("tid=%d rmem_read read_buf=[%s]\n",t->tid,read_buf);


  /* check that we read the message from the other thread */
  if (strcmp(expected_msg,read_buf)) { 
    printf("expected_msg=[%s]\n",expected_msg);
    printf("read_buf=[%s]\n",read_buf);
    WRONG(*t) 
  }

  // tid0 writes done then reads done
  // tid1 reads done then writes done
  // sequence is as follows:
  //   tid0 writes done
  //   tid1 reads done
  //   tid1 writes done
  //   tid0 reads done
  //   tid0 cleans up the resources

  if (t->tid == 0) {
    // tell the other thread I am done 
    offset = t->tid * BUFF_SIZE/NUM_THREADS;
    mrapi_rmem_write(
                     t->rmem,
                     offset+2,
                     "D",
                     0,
                     strlen("D"),
                     1,
                     1*strlen("D"),
                     1*strlen("D"),
                     &status
                     );
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
    
    // wait for the other thread to be done
    memset(read_buf,0,BUFF_SIZE);
    offset = t->tid_other * BUFF_SIZE/NUM_THREADS;
    while (strcmp(read_buf,"D")) {
      mrapi_rmem_read(
                      t->rmem,
                      offset+2,
                      read_buf,
                      0,
                      strlen("D"),
                      1,
                      1*strlen("D"),
                      1*strlen("D"),
                      &status
                      );
      if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
      //printf("tid=%d rmem_read read_buf=[%s]\n",t->tid,read_buf);	
    }
    
    /* detach from the remote memory */
    mrapi_rmem_detach(t->rmem,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
    
    /* free the remote memory */
    mrapi_rmem_delete(t->rmem,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
    
  } else {    
      // wait for the other thread to be done
    memset(read_buf,0,BUFF_SIZE);
    offset = t->tid_other * BUFF_SIZE/NUM_THREADS;
    while (strcmp(read_buf,"D")) {
      mrapi_rmem_read(
                      t->rmem,
                      offset+2,
                      read_buf,
                      0,
                      strlen("D"),
                      1,
                      1*strlen("D"),
                      1*strlen("D"),
                      &status
                      );
      if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
      //printf("tid=%d rmem_read read_buf=[%s]\n",t->tid,read_buf);	
    }
    
    // tell the other thread I am done 
    offset = t->tid * BUFF_SIZE/NUM_THREADS;
    mrapi_rmem_write(
                     t->rmem,
                     offset+2,
                     "D",
                     0,
                     strlen("D"),
                     1,
                     1*strlen("D"),
                     1*strlen("D"),
                     &status
                     );
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
    
    /* detach from the remote memory */
    mrapi_rmem_detach(t->rmem,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }
  }
 

  /* finalize mrapi (it will free any remaining rmems for us) */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG(*t) }

 
  return NULL;
}


int main () {
  int rc = 0;
  thread_data tdata[NUM_THREADS];
  int i;
  pthread_t threads[NUM_THREADS];

  // mrapi_set_debug_level(9);

  tdata[0].tid = 0;
  tdata[0].tid_other = 1;
  tdata[1].tid = 1;
  tdata[1].tid_other = 0;

  //run_thread ((void*)&tdata[0]);

  if (1) {
  rc += pthread_create(&threads[0], NULL, run_thread,(void*)&tdata[0]);
  rc += pthread_create(&threads[1], NULL, run_thread,(void*)&tdata[1]);
  
  for (i = 0; i < 2; i++) {
   pthread_join(threads[i],NULL);
  }
  }
   
  if (rc == 0) {
    printf("   Test PASSED\n");
  }

  return rc;
}
