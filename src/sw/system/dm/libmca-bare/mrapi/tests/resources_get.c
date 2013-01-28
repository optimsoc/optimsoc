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
  //  mrapi_domain_t d;
  //  mrapi_node_t n;
  // Resource related variables
  mrapi_resource_t     *root;
  mrapi_rsrc_filter_t   filter;
  mrapi_resource_t     *child;
  mrapi_resource_t     *bogus_tree;

  /* initialize */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&mrapi_status);
  if (mrapi_status  != MRAPI_SUCCESS) { WRONG }

  /* Note: Since I'm not freeing the root trees consistently, this test */
  /* has a memory leak. */
  filter = MRAPI_RSRC_DMA;
  root = mrapi_resources_get(filter, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (root != NULL) { WRONG }

  filter = 10000;  /* A bad resource filter */
  root = mrapi_resources_get(filter, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_INVALID_SUBSYSTEM) { WRONG }
  if (root != NULL) { WRONG }

  filter = MRAPI_RSRC_CPU;
  root = mrapi_resources_get(filter, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (root == NULL) { WRONG }
  if (root->number_of_children != 2) { WRONG }
  child = root->children[0];
  if (child->resource_type != CPU) { WRONG }
  child = root->children[1];
  if (child->resource_type != CPU) { WRONG }

  filter = MRAPI_RSRC_MEM;
  root = mrapi_resources_get(filter, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (root == NULL) { WRONG }
  if (root->number_of_children != 1) { WRONG }
  child = root->children[0];
  if (child->resource_type != MEM) { WRONG }

  filter = MRAPI_RSRC_CACHE;
  root = mrapi_resources_get(filter, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (root == NULL) { WRONG }
  if (root->number_of_children != 1) { WRONG }
  child = root->children[0];
  if (child->resource_type != CACHE) { WRONG }

  mrapi_resource_tree_free(&root, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }

  /* Check for improper trees */
  bogus_tree = NULL;
  mrapi_resource_tree_free(&bogus_tree, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_INVALID_TREE) { WRONG }

  /* finalize */
  mrapi_finalize (&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  printf("PASSED");
  return 0;
}
