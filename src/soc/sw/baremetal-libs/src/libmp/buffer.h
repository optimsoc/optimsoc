#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <or1k-support.h>

struct mp_cbuffer {
	uint32_t capacity; //! Buffer capacity (exponent)
	uint32_t max_msg_size;  //! Max element size (exponent)
	volatile uint32_t wr_idx; //! Next write index
	volatile uint32_t rd_idx; //! Next read index
	volatile uint8_t *data; //! Data field
	volatile uint32_t *size; //! Size field
};

typedef enum {
	MP_CBUFFER_SUCCESS = 0,
	MP_CBUFFER_GENERIC_ERROR,
	MP_CBUFFER_INVALID_PARAM,
	MP_CBUFFER_NOMEM,
	MP_CBUFFER_FULL,
	MP_CBUFFER_EMPTY
} mp_cbuffer_result_t;

mp_cbuffer_result_t mp_cbuffer_create(size_t capacity, size_t max_msg_size,
		struct mp_cbuffer** buffer);

void mp_cbuffer_destroy(struct mp_cbuffer* buf);

mp_cbuffer_result_t mp_cbuffer_reserve(struct mp_cbuffer* buf, uint32_t *idx);
void mp_cbuffer_commit(struct mp_cbuffer *buf, uint32_t idx, uint32_t size);
