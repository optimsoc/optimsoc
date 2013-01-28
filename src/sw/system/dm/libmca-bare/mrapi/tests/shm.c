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
#include <mrapi.h>

char status_buff[MRAPI_MAX_STATUS_SIZE];

char status_buff[MRAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define WRONG wrong(__LINE__);
#define SEM_KEY 0xdeadbeef
#define SHMEM_KEY 0xdeadbabe
#define LOCK_LIMIT 1

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define RESULT 1024 

unsigned* addr;
unsigned  data;


void wrong(unsigned line)
{
  mca_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mrapi_finalize(&status);
  exit(1);
}


void run () {
  mrapi_sem_hndl_t sem; /* mrapi sem handle */
  mrapi_shmem_hndl_t shmem; /* mrapi shmem handle */
  mca_status_t status;
  int i;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;

  /* initialize mrapi */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); fflush(stdout); WRONG }
  
  /* create the semaphore */
  sem = mrapi_sem_create(SEM_KEY,NULL /*attributes*/,LOCK_LIMIT,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

  /* create the shared memory and attach to it */
  shmem = mrapi_shmem_create(SHMEM_KEY,sizeof(int) /*size*/,NULL /* nodes list */,0 /* nodes list size */,NULL /*attrs*/,0 /*attrs size*/,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

  addr = mrapi_shmem_attach(shmem,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
  
  /* read, increment and write shared memory */
  for (i = 0; i < RESULT; i++) {
    mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
    *(addr) = *(addr) + 1;
    //printf("t->tid:%d data=%d\n",t->tid,(unsigned)(*(addr)));
    data = *(addr);
    mrapi_sem_unlock(sem,&status);
    if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
  }

  /* detach from the shared memory */
  mrapi_shmem_detach(shmem,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }
 
  /* finalize mrapi  */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG }

}


int main () {
 
  int rc = 0;

  mrapi_set_debug_level(1);

  run();
  if (data != RESULT) { fprintf(stderr,"FAILED: expected addr=%u, found addr=%u\n",RESULT,data); rc=1; }

  if (rc == 0) {
    printf("   Test PASSED\n");
  }
 
 
  return rc;
}
