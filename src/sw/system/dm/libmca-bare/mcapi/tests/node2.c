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
/* Test: node2
   Description: This is a simple test to make sure we can get/set node attributes.
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



#define error() ERROR(__LINE__)

int ERROR(unsigned line) {
  mcapi_status_t status;
  mcapi_finalize(&status);
  fprintf(stderr," WRONG: line=%u\n", line);
  fflush(stdout);
  exit(1);
}

int main () {
  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;
 

  mcapi_node_attributes_t node_attrs;

  /* init the node attributes */
  uint32_t attr_num = MCAPI_NODE_ATTR_TYPE_REGULAR;
  mcapi_node_attr_type_t attr=1;
  size_t attr_size = sizeof(attr);
  mcapi_node_init_attributes(&node_attrs,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED: mcapi_node_init_attributes NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  } 
  
  mcapi_node_set_attribute(&node_attrs,
                                92,
                           (void*)&attr,
                           sizeof(attr),
                           &status);
  if (status != MCAPI_ERR_ATTR_NOTSUPPORTED) {
    fprintf(stderr,"FAILED: mcapi_node_set_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  mcapi_node_set_attribute(&node_attrs,
                           attr_num,
                           (void*)&attr,
                           92,
                           &status);
  if (status != MCAPI_ERR_ATTR_SIZE) {
    fprintf(stderr,"FAILED: mcapi_node_set_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  mcapi_node_set_attribute(&node_attrs,
                           attr_num,
                           NULL,
                           sizeof(attr),
                           &status);
  if (status != MCAPI_ERR_PARAMETER) {
    fprintf(stderr,"FAILED: mcapi_node_set_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  /*
    What are the the acceptable values?
    MCAPI_ERR_ATTR_VALUE
    Incorrect attribute value.
  */
  
  /* set the node type to irregular */
  mcapi_node_set_attribute(&node_attrs,
                           attr_num,
                           (void*)&attr,
                           sizeof(attr),
                           &status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED: mcapi_node_set_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  } 
  
  mcapi_node_attr_type_t attr2=0;
  mcapi_node_get_attribute(DOMAIN,NODE,attr_num,(void*)&attr2,attr_size,&status);
  if (status != MCAPI_ERR_NODE_NOTINIT) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  /* create the node */
  mcapi_initialize(DOMAIN,NODE,&node_attrs,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED: mcapi_initialize NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  /* get the node attributes */
  mcapi_node_get_attribute(DOMAIN,NODE,attr_num,(void*)&attr2,attr_size,&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  if (attr2 != attr) {
    fprintf(stderr,"FAILED: expected attr=%d, found attr=%d\n",attr,attr2);
    error();
  }
  
  mcapi_node_get_attribute(42,NODE,attr_num,(void*)&attr2,attr_size,&status);
  if (status != MCAPI_ERR_DOMAIN_INVALID) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
 
  /* get the node attributes */
  mcapi_node_get_attribute(DOMAIN,42,attr_num,(void*)&attr2,attr_size,&status);
  if (status != MCAPI_ERR_NODE_INVALID) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  mcapi_node_get_attribute(DOMAIN,NODE,attr_num,(void*)&attr2,1,&status);
  if (status != MCAPI_ERR_ATTR_SIZE) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  mcapi_node_get_attribute(DOMAIN,NODE,42,(void*)&attr2,attr_size,&status);
  if (status != MCAPI_ERR_ATTR_NOTSUPPORTED) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
  
  mcapi_node_get_attribute(DOMAIN,NODE,attr_num,NULL,attr_size,&status);
  if (status != MCAPI_ERR_PARAMETER) {
    fprintf(stderr,"FAILED: mcapi_node_get_attribute NODE=%i, status=%s",
            NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  } 
  
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    fprintf(stderr,"FAILED 1: mcapi_finalize NODE=%i, status=%s",NODE,mcapi_display_status(status,status_buff,sizeof(status_buff)));
    error();
  }
   
  
  printf("   Test PASSED\n");
  return 0;
}
