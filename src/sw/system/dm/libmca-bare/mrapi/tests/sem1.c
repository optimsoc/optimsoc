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

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif



#define LOCK_LIMIT 1

mca_status_t mrapi_status;

#define WRONG wrong(__LINE__);
void wrong(unsigned line) {
  fprintf(stderr,"WRONG: line=%u status=%s\n", line,mrapi_display_status(mrapi_status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

int main () {

  int semkey = 0xdeaddead;
  mrapi_sem_hndl_t sem,sem2;
  mrapi_info_t version;
  mrapi_set_debug_level(3);  
  mrapi_parameters_t parms = 0;
  mrapi_boolean_t locked = MRAPI_FALSE;
  mrapi_boolean_t attr = MRAPI_TRUE;
  mrapi_sem_attributes_t attrs;

  sem=mrapi_sem_create(semkey,NULL /*atttrs */,LOCK_LIMIT,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_NODE_NOTINIT) { WRONG}

  mrapi_initialize(DOMAIN,NODE,parms,&version,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  sem=mrapi_sem_create(semkey,NULL /*atttrs */,0,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_SEM_LOCKLIMIT) { WRONG}

  sem=mrapi_sem_create(semkey,NULL /*atttrs */,MRAPI_MAX_SHARED_LOCKS+1,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_SEM_LOCKLIMIT) { WRONG}

  sem=mrapi_sem_create(semkey,NULL /*atttrs */,0xffff0000,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_SEM_LOCKLIMIT) { WRONG}

  mrapi_sem_init_attributes(&attrs,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  /* enable extended error checking */
  mrapi_sem_set_attribute(&attrs,MRAPI_ERROR_EXT,&attr,sizeof(attr),&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  sem=mrapi_sem_create(semkey,&attrs,LOCK_LIMIT,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
 
  sem2 = mrapi_sem_get(semkey,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  if (sem != sem2) { WRONG}
 
  /* already created */
  sem2=mrapi_sem_create(semkey,NULL /*atttrs */,LOCK_LIMIT,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_SEM_EXISTS) { WRONG}

  sem=mrapi_sem_get(semkey,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  
  /* invalid sem */
  sem2=mrapi_sem_get(0xb00,&mrapi_status);
  if (mrapi_status  == MRAPI_SUCCESS) { WRONG}

  locked = mrapi_sem_trylock(0xdeadbabe, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_INVALID) { WRONG}

  locked = mrapi_sem_trylock(sem, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}
  if (locked == MRAPI_FALSE) { WRONG}

  /* invalid sem */
  mrapi_sem_lock(0xb00,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_INVALID) { WRONG}

  mrapi_sem_unlock(0xb00, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_INVALID) { WRONG}
  
  mrapi_sem_unlock(sem,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

  /* not locked */
  mrapi_sem_unlock(sem,&mrapi_status);
  //if (mrapi_status == MRAPI_SUCCESS) { WRONG}
  
  mrapi_sem_delete (sem,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_LOCKED) { WRONG}
 
  mrapi_sem_lock(sem,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

  mrapi_sem_delete (sem,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

/* check edeleted error code */
  mrapi_sem_lock (sem, 0xf /* timeout */,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_DELETED) { WRONG}
  mrapi_sem_trylock (sem,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_DELETED) { WRONG}
  mrapi_sem_unlock (sem,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_DELETED) { WRONG}
  mrapi_sem_delete (sem,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_DELETED) { WRONG}
  mrapi_sem_get_attribute(sem,MRAPI_ERROR_EXT,&attr,sizeof(attr),&mrapi_status);
  if (mrapi_status != MRAPI_ERR_SEM_DELETED) { WRONG}
 
  /* invalid sem */
  mrapi_sem_delete (0xb00,&mrapi_status);
  if (mrapi_status == MRAPI_SUCCESS) { WRONG}
 
  mrapi_finalize(&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

  return 0;
}
