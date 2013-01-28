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


mca_status_t mrapi_status;

#define WRONG wrong(__LINE__);
void wrong(unsigned line) {
  fprintf(stderr,"WRONG: line=%u\n", line);
  fflush(stdout);
  mrapi_display_status(mrapi_status,status_buff,sizeof(status_buff));
  exit(1);
}

int main () {
  
  int mutexkey = 0xdeaddead;
  mrapi_mutex_hndl_t mutex,mutex2;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_key_t lock_key;
  mrapi_boolean_t locked = MRAPI_FALSE; 
  mrapi_set_debug_level(2);  
  mrapi_boolean_t attr = MRAPI_TRUE;
  mrapi_mutex_attributes_t attrs;

  mutex=mrapi_mutex_create(mutexkey,NULL /*atttrs */,&mrapi_status);
  if (mrapi_status  != MRAPI_ERR_NODE_NOTINIT) { WRONG}

  mrapi_initialize(DOMAIN,NODE,parms,&version,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  mrapi_mutex_init_attributes(&attrs,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  /* enable extended error checking */
  mrapi_mutex_set_attribute(&attrs,MRAPI_ERROR_EXT,&attr,sizeof(attr),&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}

  mutex=mrapi_mutex_create(mutexkey, &attrs,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
 
  mutex2 = mrapi_mutex_get(mutexkey,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  if (mutex != mutex2) { WRONG}
 
  /* already created */
  mutex=mrapi_mutex_create(mutexkey,NULL /*atttrs */,&mrapi_status);
  if (mrapi_status  == MRAPI_SUCCESS) { WRONG}

  mutex=mrapi_mutex_get(mutexkey,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  
  /* invalid mutex */
  mutex2=mrapi_mutex_get(0xb00,&mrapi_status);
  if (mrapi_status  == MRAPI_SUCCESS) { WRONG}

  
  locked = mrapi_mutex_trylock(0xdeadbabe,&lock_key, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_INVALID) { WRONG}

  locked = mrapi_mutex_trylock(mutex,&lock_key, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}
  if (locked == MRAPI_FALSE) { WRONG}

  /* invalid mutex */
  mrapi_mutex_lock(0xb00,&lock_key,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
  if (mrapi_status == MRAPI_SUCCESS) { WRONG}
  
  mrapi_mutex_unlock(mutex,&lock_key,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

  /* bad handle */
  mrapi_mutex_unlock(0xdeadbabe,&lock_key,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_INVALID) { WRONG}

  /* not locked */
  mrapi_mutex_unlock(mutex,&lock_key,&mrapi_status);
  //if (mrapi_status == MRAPI_SUCCESS) { WRONG}
 
  mrapi_mutex_lock(mutex,&lock_key,MRAPI_TIMEOUT_INFINITE /*timeout*/, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}
 
  mrapi_mutex_delete (mutex,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}
 
  /* check edeleted error code */
  mrapi_mutex_lock (mutex,&lock_key, 0xf /* timeout */,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_DELETED) { WRONG}
  mrapi_mutex_trylock (mutex,&lock_key,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_DELETED) { WRONG}
  mrapi_mutex_unlock (mutex,&lock_key,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_DELETED) { WRONG}
  mrapi_mutex_delete (mutex,&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_DELETED) { WRONG}
  mrapi_mutex_get_attribute(mutex,MRAPI_ERROR_EXT,&attr,sizeof(attr),&mrapi_status);
  if (mrapi_status != MRAPI_ERR_MUTEX_DELETED) { WRONG}

  /* invalid mutex */
  mrapi_mutex_delete (0xb00,&mrapi_status);
  if (mrapi_status == MRAPI_SUCCESS) { WRONG}
 
  mrapi_finalize(&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG}

  return 0;
}
