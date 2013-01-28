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

#ifndef NODE
#define NODE 2 
#endif

#ifndef DOMAIN
#define DOMAIN 1 
#endif

mrapi_status_t status;

#define ERR(msg) err_func(msg, __LINE__);
void err_func(char *err_msg, unsigned line) {
/*   fprintf(stderr,"ERROR: %s\n", line); */
  fprintf(stderr,"ERROR: %s, line = %u status=%s\n", err_msg, line, mrapi_display_status(status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

int main () {
  mrapi_parameters_t parms = 0;
  mrapi_info_t version;
  mrapi_resource_t* mem_root;
  mrapi_shmem_hndl_t shmem_hndl;
  mrapi_shmem_attributes_t shmem_attributes;
  int i;
  uint32_t addr;

  /* initialize */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&status);
  if (status != MRAPI_SUCCESS) { ERR("Unable to initialize"); }

  // get the metadata resource tree (filtered for memory only)
  mem_root = mrapi_resources_get (MRAPI_RSRC_MEM,&status);
  if (status != MRAPI_SUCCESS) { ERR("Unable to get resource tree");}

  // find the desired memory in the metadata resource tree
  for (i = 0; i < mem_root->number_of_children; i++) {
    mrapi_resource_get_attribute (
				  mem_root->children[i],
				  MRAPI_RSRC_MEM_BASEADDR,
				  (void*) &addr,
				  sizeof(mrapi_rsrc_mem_attrs),
				  &status);
    if (status != MRAPI_SUCCESS) { ERR ("Unable to get resource attr");}

    if (addr == 0xfffff000) {
      // we've found the resource for the region we want to use

      // set up the shared memory resource attribute with the metadata
      mrapi_shmem_init_attributes (&shmem_attributes, &status);
      if (status != MRAPI_SUCCESS) { ERR ("Unable to init shmem attrs");}

      mrapi_shmem_set_attribute (&shmem_attributes,
				 MRAPI_SHMEM_RESOURCE,
				 mem_root->children[i],
				 sizeof(*(mem_root->children[i])),
				 &status);
      if (status != MRAPI_SUCCESS) { ERR("Unable to set shmem attrs");}

      // create the shared memory
      shmem_hndl = mrapi_shmem_create (MRAPI_SHMEM_ID_ANY, 
				       1024 /* size */, 
				       NULL /*share with all nodes*/, 
				       0 /*nodes_size*/, 
				       &shmem_attributes, 
				       sizeof(shmem_attributes), 
				       &status);
      if (status != MRAPI_SUCCESS) { ERR("Unable to create shmem");}

      // delete the newly created 
      mrapi_shmem_delete (shmem_hndl, &status);
      if (status != MRAPI_SUCCESS) { ERR("Unable to delete shmem");}

      break;
    }
  }

  mrapi_finalize (&status);
  if (status != MRAPI_SUCCESS) { ERR("Unable to finalize"); }
  printf("PASSED\n");
  return 0;
}
