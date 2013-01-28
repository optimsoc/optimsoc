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
#include <pthread.h>
#include <string.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

#define NUM_MSGS 10

#define WRONG() wrong(__LINE__);
#define RMEM_KEY 0xdeadbabe
#define BUFF_SIZE 1024

#define MSG_SIZE 16

#define TIMEOUT 0xffff

void wrong(unsigned line)
{
  mca_status_t status;
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mrapi_finalize(&status);
  exit(1);
}


mrapi_boolean_t run () {
  mca_status_t status;
  int offset;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  char buf[BUFF_SIZE]; // the remote memory buffer 
  char msg [MSG_SIZE];  // what we will write from
  char read_buf [MSG_SIZE];  // what we will read into
  char read_buf2[NUM_MSGS][MSG_SIZE];
  mrapi_rmem_hndl_t rmem;
  mrapi_boolean_t rc = MRAPI_FALSE;
  int i;
  mrapi_request_t*  read_requests[NUM_MSGS];
  mrapi_request_t*  write_requests[NUM_MSGS];
  size_t size;

  
  /* initialize mrapi */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  /* create or get the remote memory and attach to it */
  rmem = mrapi_rmem_create(RMEM_KEY,&buf,MRAPI_RMEM_DUMMY,NULL /*attrs*/,sizeof(buf) /*size*/,&status);
  if (status == MRAPI_ERR_RMEM_EXISTS) {
    printf("rmem exists, try getting it from the other node");
    rmem = mrapi_rmem_get(RMEM_KEY,MRAPI_RMEM_DUMMY,&status);
  }
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  mrapi_rmem_attach(rmem,MRAPI_RMEM_DUMMY,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }
  
  /* read and write remote memory */
  sprintf(msg,"Hello World\n");
  offset = 0;
  mrapi_rmem_write(
                   rmem,
                   offset,
                   msg,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg),
                   &status
                   );
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  mrapi_rmem_read(
                   rmem,
                   offset,
                   read_buf,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg),
                   &status
                   );
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

for (i=0; i<NUM_MSGS; i++) {
  write_requests[i] = (mrapi_request_t*)malloc (sizeof(mrapi_request_t));
  mrapi_rmem_write_i(
                   rmem,
                   offset+i*(strlen(msg)),
                   msg,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg),
                   write_requests[i],
                   &status
                   );
   read_requests[i] = (mrapi_request_t*)malloc (sizeof(mrapi_request_t));
   mrapi_rmem_read_i(
                   rmem,
                   offset+i*(strlen(msg)),
                   read_buf2[i],
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg),
                   read_requests[i],
                   &status
                   );
  }  
  i = mrapi_wait_any(
            NUM_MSGS,
            read_requests[0],
            &size,
            TIMEOUT,
            &status
  );
  if ((i == MRAPI_NONE) || (status != MRAPI_SUCCESS)) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }
  printf("i=%d read_buf2[i] = [%s]\n",i,read_buf2[i]);
 
  /* read and write with a stride < bytes_per_access */
  mrapi_rmem_write(
                   rmem,
                   offset,
                   msg,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg)-1,
                   1*strlen(msg),
                   &status
                   );
  if (status != MRAPI_ERR_RMEM_STRIDE) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  mrapi_rmem_read(
                   rmem,
                   offset,
                   read_buf,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg)-1,
                   1*strlen(msg),
                   &status
                   );
  if (status != MRAPI_ERR_RMEM_STRIDE) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  mrapi_rmem_write(
                   rmem,
                   offset,
                   msg,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg)-1,
                   &status
                   );
  if (status != MRAPI_ERR_RMEM_STRIDE) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  mrapi_rmem_read(
                   rmem,
                   offset,
                   read_buf,
                   offset,
                   strlen(msg),
                   1,
                   1*strlen(msg),
                   1*strlen(msg)-1,
                   &status
                   );
  if (status != MRAPI_ERR_RMEM_STRIDE) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  /* detach from the remote memory */
  mrapi_rmem_detach(rmem,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  /* free the remote memory */
  mrapi_rmem_delete(rmem,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }

  /* finalize mrapi (it will free any remaining sems & rmems for us) */
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); WRONG() }


  /* check that we read the correct message */
  if (strcmp(msg,read_buf)) { rc = MRAPI_FALSE; }

  for (i = 0; i < NUM_MSGS; i++) {
    free(read_requests[i]);
    free(write_requests[i]);
  }

  return rc;
}


int main () {
 
  int rc = 0;
  

  mrapi_set_debug_level(9);
  
  rc = run();
 
  if (rc == 0) {
    printf("   Test PASSED\n");
  }
 
  return rc;
}
