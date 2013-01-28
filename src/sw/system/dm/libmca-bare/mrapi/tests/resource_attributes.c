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
#include <unistd.h>

#ifndef NODE
#define NODE 2 
#endif

#ifndef DOMAIN
#define DOMAIN 1 
#endif

#define LOCK_LIMIT 1

mca_status_t mrapi_status;

extern void mrapi_impl_increment_cache_hits(mrapi_resource_t *resource, int increment);
extern void mrapi_impl_trigger_rollover(uint16_t index);

/* Callbacks */
mrapi_boolean_t rollover = MRAPI_FALSE;
void l3cache_hits_rollover(void) {
  rollover = MRAPI_TRUE;
}
mrapi_boolean_t buffer_nearly_full = MRAPI_FALSE;
void crossbar_buffer_near_full(mrapi_event_t event) {
  buffer_nearly_full = MRAPI_TRUE;
}

#define WRONG wrong(__LINE__);
void wrong(unsigned line) {
  fprintf(stderr,"WRONG: line=%u status=%s\n", line,mrapi_display_status(mrapi_status,status_buff,sizeof(status_buff)));
  fflush(stdout);
  exit(1);
}

int main () {
  mrapi_parameters_t parms = 0;
  mrapi_info_t version;
  // Resource related variables
  mrapi_resource_t     *root;
  mrapi_rsrc_filter_t   filter;
  mrapi_resource_t     *ddr;
  mrapi_resource_t     *core;
  mrapi_resource_t     *crossbar;
  mrapi_resource_t     *l3cache;
  void                 *attr_val = NULL;
  size_t                attr_size;
  int i;

  mrapi_set_debug_level(3);

  /* initialize */
  mrapi_initialize(DOMAIN,NODE,parms,&version,&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }

  /* Get several attributes of different nodes */
  filter   = MRAPI_RSRC_CROSSBAR;
  root     = mrapi_resources_get(filter, &mrapi_status);
  crossbar = root->children[0];
  uint16_t crossbar_number_of_ports = 10000;
  attr_size = 2; /* Size of integer in bytes */
  mrapi_resource_get_attribute(crossbar, 0, (void *)&crossbar_number_of_ports, 
			       attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (crossbar_number_of_ports != 8) { WRONG }

  filter = MRAPI_RSRC_MEM;
  root   = mrapi_resources_get(filter, &mrapi_status);
  ddr    = root->children[0];
  /* Check for a bad attribute number */
  mrapi_resource_get_attribute(ddr, 1000, attr_val, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_ATTR_NUM) { WRONG }
  /* Get a valid attribute */
  uint16_t ddr_size;
  attr_size = 2;
  mrapi_resource_get_attribute(ddr, 0, (void *)&ddr_size, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (ddr_size != 1024) { WRONG }

  filter = MRAPI_RSRC_CPU;
  root = mrapi_resources_get(filter, &mrapi_status);
  core = root->children[1];
  uint32_t core_id;
  attr_size = 4;
  mrapi_resource_get_attribute(core, 0, (void *)&core_id, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (core_id != 400000002) { WRONG }

  filter = MRAPI_RSRC_CACHE;
  root = mrapi_resources_get(filter, &mrapi_status);
  l3cache = root->children[0];
  uint32_t cache_hits;
  attr_size = 4;
  mrapi_resource_get_attribute(l3cache, 1, (void *)&cache_hits, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_NOTSTARTED) { WRONG }

  /* */
  /* Test the start, reset, and stop the dynamic attributes */
  /* */
  /* Test the dynamic start */
  /* Start the L3 cache hits variable */
  mrapi_dynamic_attribute_start(l3cache, 1, &l3cache_hits_rollover, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  mrapi_resource_get_attribute(l3cache, 1, (void *)&cache_hits, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (cache_hits != 0) { WRONG }
  /* Test the backdoor that artificially increments the number of cache hits */
  mrapi_impl_increment_cache_hits(l3cache, 100);
  mrapi_resource_get_attribute(l3cache, 1, (void *)&cache_hits, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (cache_hits != 100) { WRONG }
  /* Use a backdoor to cause a rollover event */
  if (rollover != MRAPI_FALSE) { WRONG }
  mrapi_impl_trigger_rollover(0);  /* Since L3 cache hits was started first, index is 0 */
  if (rollover != MRAPI_TRUE) { WRONG }

  /* Test the dynamic reset */
  /* Try a couple of bad requests */
  mrapi_dynamic_attribute_reset(l3cache, 100000, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_ATTR_NUM) { WRONG }
  mrapi_dynamic_attribute_reset(l3cache, 0, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_NOTDYNAMIC) { WRONG }
  /* Try reseting the number of cache hits */
  mrapi_dynamic_attribute_reset(l3cache, 1, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  mrapi_resource_get_attribute(l3cache, 1, (void *)&cache_hits, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (cache_hits != 0) { WRONG }

  /* Test the dynamic stop */
  /* Try a couple of bad requests */
  mrapi_dynamic_attribute_stop(l3cache, 100000, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_ATTR_NUM) { WRONG }
  mrapi_dynamic_attribute_stop(l3cache, 0, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_NOTDYNAMIC) { WRONG }
  /* Test the cache hits */
  mrapi_impl_increment_cache_hits(l3cache, 100);
  mrapi_dynamic_attribute_stop(l3cache, 1, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  mrapi_resource_get_attribute(l3cache, 1, (void *)&cache_hits, attr_size, &mrapi_status);
  if (mrapi_status != MRAPI_ERR_RSRC_NOTSTARTED) { WRONG }

  /* Test the callback mechanism */
  mrapi_event_t crossbar_event = MRAPI_EVENT_CROSSBAR_BUFFER_OVER_80PERCENT;
  unsigned int crossbar_frequency = 3;
  mrapi_resource_register_callback(crossbar_event, crossbar_frequency,
				   &crossbar_buffer_near_full, &mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  if (buffer_nearly_full != MRAPI_FALSE) { WRONG }
  /* Sleep 3 times to allow the 3 alarms to trigger the crossbar callback */
  for (i = 0; i < 3; i++) {
    sleep(2);
  }
  if (buffer_nearly_full != MRAPI_TRUE) { WRONG }

  /* finalize */
  mrapi_finalize(&mrapi_status);
  if (mrapi_status != MRAPI_SUCCESS) { WRONG }
  printf("PASSED");
  return 0;
}
