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
/* Test: node1
   Description: This is a simple test of initialize, finalize and get_node_num.
*/


#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>


#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif


#define PORT_NUM 15

#define BUFF_SIZE 128

void check_display_status (mca_status_t s,char* e) {
  char buff [BUFF_SIZE];
  char buff2[BUFF_SIZE];
  sprintf(buff2,"%s",e);
  sprintf(buff,"%s",mcapi_display_status(s,status_buff,sizeof(status_buff)));
  if (strcmp(buff,buff2) != 0) {
    fprintf(stderr,"FAILED: mcapi_display_status(%i) returned %s, expected %s\n",(int)s,mcapi_display_status(s,status_buff,sizeof(status_buff)),e);
    exit(-1);
  }
}



int main () {
  mca_status_t status;
mcapi_param_t parms;
  mcapi_info_t version;
  int node_num;

  status = 41;
  printf("s=%s",mcapi_display_status(status,status_buff,sizeof(status_buff)));

  mcapi_set_debug_level(6);

  /* try to get my node_num before calling initialize */
  node_num = mcapi_node_id_get (&status);
  if (status != MCAPI_ERR_NODE_NOTINIT) {
    fprintf(stderr,"FAILED 2: mcapi_get_node_num NODE=%i, status=%s",node_num,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED 1: mcapi_initialize NODE=%i, status=%s",NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

#if 0
  /* check eparam */
  mcapi_initialize(DOMAIN,NODE+1,NULL,&parms,NULL,&status);
  if (status != MCAPI_ERR_PARAMETER) {
    fprintf(stderr,"FAILED 1: mcapi_initialize expected MCAPI_EPARAM, received: %s",mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
#endif

  /* get my node_num */
  node_num = mcapi_node_id_get (&status);
  if (node_num != NODE) {
    fprintf(stderr,"FAILED 2: mcapi_get_node_num NODE=%i, status=%s",node_num,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  
  /* try to call initialize again, should error */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status == MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED 3: mcapi_initialize NODE=%i, status=%s",NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED 1: mcapi_finalize NODE=%i, status=%s",NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }




  printf("   Test PASSED\n");
  return 0;
}
