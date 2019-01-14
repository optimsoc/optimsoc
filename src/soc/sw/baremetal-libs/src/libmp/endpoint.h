#pragma once

#include <optimsoc-baremetal.h>
#include "include/optimsoc-mp.h"

// The endpoint handle points either to a local or a remote endpoint
struct endpoint_handle {
    enum { LOCAL, REMOTE } type;
    uint32_t domain;
    uint32_t node;
    uint32_t port;
    union {
    	struct endpoint_local *local;
    	struct endpoint_remote *remote;
    } ptr;
};

typedef enum { CONNECTIONLESS = 0, CONNECTIONORIENTED = 1 } endpoint_type;

struct endpoint_local {
	struct endpoint_remote *connected;
	struct mp_cbuffer *recv_buffer;
	struct mp_cbuffer *send_buffer;
	uint8_t padding[4];
};

struct endpoint_remote {
	uint32_t tile;
	uint32_t addr;
	uint32_t credit; // chan only
	uint32_t curidx; // chan only
	uint32_t capacity; // chan only
	uint32_t buffer; // chan only
};

optimsoc_mp_result_t endpoint_create(struct endpoint_handle ** eph,
		uint32_t domain, uint32_t node, uint32_t port,
		uint32_t buffer_size, uint32_t max_size,
		uint32_t attr);

optimsoc_mp_result_t endpoint_destroy(struct endpoint_handle *eph);

void endpoint_print(struct endpoint_handle *ep, char* pre);
