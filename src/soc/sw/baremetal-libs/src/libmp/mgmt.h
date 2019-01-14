#pragma once

#include "include/optimsoc-mp.h"

optimsoc_mp_result_t mgmt_init(optimsoc_mp_mgmt_type_t type);

optimsoc_mp_result_t mgmt_register(struct endpoint_handle* ep);

optimsoc_mp_result_t mgmt_get(struct endpoint_handle ** eph,
		uint32_t domain, uint32_t node, uint32_t port, int local);

void mgmt_print_db();
