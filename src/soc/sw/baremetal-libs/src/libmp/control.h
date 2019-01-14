#pragma once

#include <optimsoc-baremetal.h>

#include "endpoint.h"



void control_init();

optimsoc_mp_result_t control_get_endpoint(struct endpoint_remote ** ep, uint32_t tile,
		uint32_t domain, uint32_t node, uint32_t port);

optimsoc_mp_result_t control_msg_alloc(struct endpoint_handle *eph, uint32_t *addr, uint32_t *index);
void control_msg_data(struct endpoint_handle *ep, uint32_t address, uint32_t* buffer,
                      uint32_t size);

void control_msg_final(struct endpoint_handle *ep, uint32_t index, uint32_t size);

