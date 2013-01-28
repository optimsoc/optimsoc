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
// The basic signal handling test.  This is a single non-threaded process and the app does have it's own signal handler.
//  Fix me: a checker should be added to verify that the following events occur in order:
//    1) mrapi registers it's signal handler
//    2) mcapi registers it's signal handler
//    3) the app registers it's signal handler
//    4) the app catches the signal and re-raises it
//    5) mcapi catches the signal and re-raises it
//    6) mrapi catches the signal and re-raises it

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <signal.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif

extern void register_handlers();

int main () {
  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;

  mcapi_set_debug_level(1);
 
  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED 1: mcapi_initialize NODE=%i, status=%s",NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  
  register_handlers();
 
  raise(SIGINT);
  
 return 0;
}

