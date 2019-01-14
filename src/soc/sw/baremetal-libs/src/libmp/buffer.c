#include "buffer.h"

#include <stdio.h>
#include <assert.h>

mp_cbuffer_result_t mp_cbuffer_create(size_t capacity, size_t max_msg_size,
		struct mp_cbuffer** buffer) {
	if (buffer == 0) {
		return MP_CBUFFER_INVALID_PARAM;
	}
	*buffer = 0;

	struct mp_cbuffer *buf = calloc(1, sizeof(struct mp_cbuffer));
	if (buf == 0) {
		return MP_CBUFFER_NOMEM;
	}

	buf->capacity = (uint32_t) capacity;
	buf->max_msg_size = (uint32_t) max_msg_size;
	buf->wr_idx = 0;
	buf->rd_idx = 0;

	buf->data = calloc(1<<capacity, 1<<max_msg_size);
	assert(((uint32_t) buf->data & 0x3) == 0); // Check that "suitably aligned"
	if (buf->data == 0) {
		free(buf);
		return MP_CBUFFER_NOMEM;
	}

	buf->size = calloc(1<<capacity, sizeof(uint32_t));
	assert(((uint32_t) buf->size & 0x3) == 0); // Check that "suitably aligned"
	if (buf->size == 0) {
		free((void*) buf->data);
		free(buf);
		return MP_CBUFFER_NOMEM;
	}

	*buffer = buf;
	return MP_CBUFFER_SUCCESS;
}

void mp_cbuffer_destroy(struct mp_cbuffer* buf) {
	free((void*)buf->size);
	free((void*)buf->data);
	free(buf);
}

mp_cbuffer_result_t mp_cbuffer_reserve(struct mp_cbuffer* buf, uint32_t *idx) {
	uint32_t cur_idx, new_idx, rd_idx;
	do {
		cur_idx = buf->wr_idx;
		*idx = cur_idx % (1 << buf->capacity);
		rd_idx = buf->rd_idx;
		if ((*idx == (rd_idx % (1 << buf->capacity))) &&
				(rd_idx != cur_idx)) {
			return MP_CBUFFER_FULL;
		}

	    if (buf->size[*idx] != 0) return MP_CBUFFER_FULL;

	    new_idx = cur_idx + 1;
	} while (or1k_sync_cas((void*) &buf->wr_idx, cur_idx, new_idx) != cur_idx);

	return MP_CBUFFER_SUCCESS;
}
void mp_cbuffer_commit(struct mp_cbuffer *buf, uint32_t idx, uint32_t size) {
  buf->size[idx] = size;
}

mp_cbuffer_result_t mp_cbuffer_push(struct mp_cbuffer *buf, uint8_t *data,
		     uint32_t size, uint32_t *idx) {
  mp_cbuffer_result_t res;

  res = mp_cbuffer_reserve(buf, idx);
  if (res != MP_CBUFFER_SUCCESS)
  	return res;

  memcpy((uint8_t*) buf->data + *idx * (1 << buf->max_msg_size), data, size);

  mp_cbuffer_commit(buf, *idx, size);

  return MP_CBUFFER_SUCCESS;
}

mp_cbuffer_result_t mp_cbuffer_read(struct mp_cbuffer *buf, uint32_t *idx) {
  uint32_t cur_idx, new_idx;

  do {
    cur_idx = buf->rd_idx;
    new_idx  = cur_idx + 1;
    *idx = cur_idx % (1 << buf->capacity);
    if (buf->size[*idx]==0) return MP_CBUFFER_EMPTY;
  } while(or1k_sync_cas((uint32_t*) &buf->rd_idx, cur_idx, new_idx) != cur_idx);

  return MP_CBUFFER_SUCCESS;
}

void mp_cbuffer_free(struct mp_cbuffer *buf, uint32_t idx) {
  buf->size[idx] = 0;
}

int mp_cbuffer_pop(struct mp_cbuffer *buf, uint8_t *data, uint32_t *size) {
  uint32_t idx;
  mp_cbuffer_result_t res;
  res = mp_cbuffer_read(buf, &idx);
  if (res != MP_CBUFFER_SUCCESS) {
	  return res;
  }
  *size = buf->size[idx];

  memcpy(data, (uint8_t*) buf->data + idx*(1 << buf->max_msg_size), *size);

  mp_cbuffer_free(buf, idx);

  return MP_CBUFFER_SUCCESS;
}

