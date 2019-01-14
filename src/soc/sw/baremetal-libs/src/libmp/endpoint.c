#include "include/optimsoc-mp.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include "endpoint.h"
#include "buffer.h"
#include "trace.h"

// #define ALLOC(t) aligned_alloc(sizeof(t), sizeof(t)) C11
#define ALLOC(t) memalign(sizeof(t), sizeof(t))

optimsoc_mp_result_t endpoint_create(struct endpoint_handle ** handle,
		uint32_t domain, uint32_t node, uint32_t port,
		uint32_t buffer_size, uint32_t max_size, uint32_t attr) {
	trace_mp_ll_ep_create_enter(domain, node, port, buffer_size, max_size, attr);

	optimsoc_mp_result_t reterr = OPTIMSOC_MP_ERR_GENERIC;

	struct endpoint_local *ep = ALLOC(struct endpoint_local);
	if (!ep) {
		return OPTIMSOC_MP_ERR_NO_MEMORY;
	}
	memset(ep, 0, sizeof(struct endpoint_local));

	if (attr & OPTIMSOC_MP_EP_BUFFERED) {
		mp_cbuffer_result_t ret = mp_cbuffer_create(buffer_size, max_size, &ep->send_buffer);
		if (ret != MP_CBUFFER_SUCCESS) {
			reterr = OPTIMSOC_MP_ERR_NO_MEMORY;
			goto cleanup;
		}
	}

	if ((attr & OPTIMSOC_MP_EP_NORECV) == 0) {
		mp_cbuffer_result_t ret = mp_cbuffer_create(buffer_size, max_size, &ep->recv_buffer);
		if (ret != MP_CBUFFER_SUCCESS) {
			reterr = OPTIMSOC_MP_ERR_NO_MEMORY;
			goto cleanup;
		}
	}

	struct endpoint_handle *eph = calloc(1, sizeof(struct endpoint_handle));
	if (!eph) {
		reterr = OPTIMSOC_MP_ERR_NO_MEMORY;
		goto cleanup;
	}

	eph->type = LOCAL;
	eph->domain = domain;
	eph->node = node;
	eph->port = port;
	eph->ptr.local = ep;

	*handle = eph;

	trace_mp_ll_ep_create_leave(eph);

	return OPTIMSOC_MP_SUCCESS;

cleanup:
	if (ep->send_buffer) mp_cbuffer_destroy(ep->send_buffer);
	if (ep->recv_buffer) mp_cbuffer_destroy(ep->recv_buffer);
	free(ep);

	return reterr;
}

optimsoc_mp_result_t endpoint_destroy(struct endpoint_handle *eph) {
	trace_mp_ll_ep_destroy_enter(eph);
	if (!eph) {
		return OPTIMSOC_MP_ERR_INVALID_PARAMETER;
	}

	if (eph->type == LOCAL) {
		if (eph->ptr.local->send_buffer) mp_cbuffer_destroy(eph->ptr.local->send_buffer);
		if (eph->ptr.local->recv_buffer) mp_cbuffer_destroy(eph->ptr.local->recv_buffer);
		free(eph->ptr.local);
	} else {
		free(eph->ptr.remote);
	}

	free(eph);

	trace_mp_ll_ep_destroy_leave();

	return OPTIMSOC_MP_SUCCESS;
}

void endpoint_print(struct endpoint_handle *ep, char* pre) {
	printf("%s(%p) domain=%lu node=%lu port=%lu\n", pre, ep, ep->domain, ep->node, ep->port);
	if (ep->type == LOCAL) {
		printf("%s  LOCAL (%p)\n", pre, ep->ptr.local);
		printf("%s    send_buffer=(%p) recv_buffer=(%p)\n", pre, ep->ptr.local->send_buffer, ep->ptr.local->recv_buffer);
		if (ep->ptr.local->connected) {
			printf("%s    connected\n", pre);
		}
	} else {
		printf("%s  REMOTE (%p)\n", pre, ep->ptr.remote);
		printf("%s    tile=%lu addr=%p\n", pre, ep->ptr.remote->tile, (void*) ep->ptr.remote->addr);
	}
}
