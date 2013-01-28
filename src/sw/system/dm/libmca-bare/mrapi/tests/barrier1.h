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

/*
  NOTE: The called needs to create a mutex for the barrier to use and
   pass in the handle.

  NOTE: it's up to the caller to handle resetting the barrier if they want
   to use it multiple times.  The safest way to do this is to have two
   barriers.  This avoids the case where the first guy through zeroes the
   barrier before the others have a chance to notice.  
*/

#ifndef WRONG
#define WRONG(x) wrong(x,__LINE__);
void wrong(mca_status_t status,unsigned line)
{
  fprintf(stderr,"WRONG: line=%u status=%s",line,mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}
#endif

void barrier (mrapi_mutex_hndl_t mutex,volatile int* barrier,int count);

void barrier (mrapi_mutex_hndl_t mutex,volatile int* barrier,int count) {
  mrapi_key_t lock_key;
  volatile int who_is_here = 0;
  mrapi_status_t status;
  mrapi_node_t node_num = mrapi_node_id_get(&status); 
  // synchronize
  // I'm here
  mrapi_mutex_lock(mutex,&lock_key,MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  *barrier = *barrier+1;
  mrapi_mutex_unlock(mutex,&lock_key,&status);
  if (status != MRAPI_SUCCESS) { WRONG(status) }
  // Wait for everyone else
  while (who_is_here != count) {
    mrapi_mutex_lock(mutex,&lock_key,MRAPI_TIMEOUT_INFINITE /*timeout*/, &status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
    who_is_here = *barrier;
    printf("node=%d, barrier=%d\n",node_num,who_is_here);
    mrapi_mutex_unlock(mutex,&lock_key,&status);
    if (status != MRAPI_SUCCESS) { WRONG(status) }
  }
}
