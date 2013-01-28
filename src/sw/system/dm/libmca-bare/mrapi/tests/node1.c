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
#define NODE 2 
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
  mrapi_parameters_t parms = 0;
  mrapi_info_t version;
  mrapi_domain_t d;
  mrapi_node_t n;
 

  /* initialize */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  
  n = mrapi_node_id_get (&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  if (n != NODE) { WRONG }

  d = mrapi_domain_id_get (&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  printf ("************************** node=%d, domain=%d\n",n,d);
  if (d != DOMAIN) { WRONG }


  /* finalize */
  mrapi_finalize (&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG}
  printf("PASSED");
  return 0;  
}
