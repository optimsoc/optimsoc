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
/* Tests various error conditions for remote memory */


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



#define RMEM_KEY 0xdeadbabe
#define BUF_SIZE 10


#define FINALIZE finalize(__LINE__);

void finalize(int line) {
  mrapi_status_t status;

  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); }

  printf("   Test FAILED line=%d\n",line);
  exit(1);
}



int main() {
  mrapi_status_t status; 
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_rmem_hndl_t rmem = 0;
  int rc = 0;
  char buf[BUF_SIZE];
  int offset = 0;

  /* create before initialize */
  rmem=mrapi_rmem_create(RMEM_KEY,&buf,MRAPI_RMEM_DUMMY,NULL /*attrs*/,sizeof(buf) /*size*/,&status);
  if (status != MRAPI_ERR_NODE_NOTINIT) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE}
  
  /* get before initialize*/
  rmem = mrapi_rmem_get(RMEM_KEY,MRAPI_RMEM_DUMMY,&status);
  if (status != MRAPI_ERR_NODE_NOTINIT) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }

 /* initialize mrapi */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }
  
  /* create w/ invalid type */
  rmem=mrapi_rmem_create(RMEM_KEY,&buf,42,NULL /*attrs*/,sizeof(buf) /*size*/,&status);
  if (status != MRAPI_ERR_RMEM_TYPENOTVALID) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }
  
  /* get w/ invalid type */
  rmem = mrapi_rmem_get(RMEM_KEY,42,&status);
  if (status != MRAPI_ERR_RMEM_TYPENOTVALID) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }

  /* get an rmem that doesn't exist */
  rmem = mrapi_rmem_get(RMEM_KEY,MRAPI_RMEM_DUMMY,&status);
  if (status != MRAPI_ERR_RMEM_ID_INVALID) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }

  /* atttach w/ a bad handle */
  mrapi_rmem_attach(rmem,MRAPI_RMEM_DUMMY,&status);
  if (status != MRAPI_ERR_RMEM_INVALID)  { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }
  
  /* read w/ a bad handle */
  mrapi_rmem_read(
                  rmem,
                  offset,
                  buf,
                  offset,
                  BUF_SIZE,
                  1,
                  BUF_SIZE,
                  BUF_SIZE,
                  &status
                  );  
  if (status != MRAPI_ERR_RMEM_INVALID)  { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }
  
  /* write w/ a bad handle */
  mrapi_rmem_write(
                   rmem,
                   offset,
                   buf,
                   offset,
                   BUF_SIZE,
                   1,
                   BUF_SIZE,
                   BUF_SIZE,
                   &status
                   );
  if (status != MRAPI_ERR_RMEM_INVALID)  { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; FINALIZE }

  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) { printf("status=%s",mrapi_display_status(status,status_buff,sizeof(status_buff))); rc = 1; }

  if (rc != 0) {
    printf("   Test FAILED\n");
  } else {
    printf("   Test PASSED\n");
  }
 
 
  return rc;
}
