#include "include/optimsoc-mp.h"

#include "control.h"
#include "endpoint.h"
#include "mgmt.h"
#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/*
 * All message passing functions are entered here from the software. The general
 * handling of the protocol is implemented here, but all accesses to the
 * endpoints need to be done via endpoint_* and all control messages need to be
 * send via control_*.
 */

enum {
    MP_OFFLOAD_NONE = 0,
	MP_OFFLOAD_RMA = 1,
	MP_OFFLOAD_NAMP = 2
} _optimsoc_mp_offload;

uint32_t _mp_trace_config;

optimsoc_mp_result_t optimsoc_mp_initialize(optimsoc_mp_mgmt_type_t mgmt_type) {
	optimsoc_mp_result_t ret;
    // Initialize endpoints
    ret = mgmt_init(mgmt_type);
    if (ret != OPTIMSOC_MP_SUCCESS) {
    	return ret;
    }

    // Initialize control plane
    control_init();

    _optimsoc_mp_offload = MP_OFFLOAD_NONE;
    _mp_trace_config = 0;

    return 0;
}

char* optimsoc_mp_error_string(int errno) {
    char* str;

    switch(errno) {
    default:
        str = "unknown error";
        break;
    }

    return str;
}

optimsoc_mp_result_t optimsoc_mp_endpoint_create(optimsoc_mp_endpoint_handle *endpoint,
		uint32_t domain, uint32_t node, uint32_t port,
		uint32_t buffer_size, size_t msg_size, optimsoc_endpoint_attr_t attr) {
	optimsoc_mp_result_t ret;

	trace_mp_endpoint_create_enter(domain, node, port, buffer_size, msg_size, attr);

	// Convert to exponent value and validate parameters are power of two
	uint32_t btmp = buffer_size;
	uint32_t mtmp = msg_size;
	uint32_t bsize = 0;
	uint32_t msize = 0;
	while (btmp >>= 1) bsize++;
	while (mtmp >>= 1) msize++;
	if (((1 << bsize) != buffer_size) || ((1 << msize) != msg_size)
			|| (msg_size < 4)) {
		return OPTIMSOC_MP_ERR_INVALID_PARAMETER;
	}

	// Create local endpoint
	ret = endpoint_create(endpoint, domain, node, port, bsize, msize, attr);
	if (ret != OPTIMSOC_MP_SUCCESS) {
		return ret;
	}

	// Make known via the management system
	ret = mgmt_register(*endpoint);
	if (ret != OPTIMSOC_MP_SUCCESS) {
		endpoint_destroy(*endpoint);
		return ret;
	}

	trace_mp_endpoint_create_leave(*endpoint);

    return 0;
}

optimsoc_mp_result_t optimsoc_mp_endpoint_get(optimsoc_mp_endpoint_handle *endpoint,
        uint32_t domain, uint32_t node, uint32_t port) {
	optimsoc_mp_result_t ret;

	trace_mp_endpoint_get_enter(domain, node, port);

	ret = mgmt_get(endpoint, domain, node, port, 0);

	trace_mp_endpoint_get_leave(*endpoint);

	return ret;
}

optimsoc_mp_result_t optimsoc_mp_msg_send(optimsoc_mp_endpoint_handle from,
		optimsoc_mp_endpoint_handle to, uint32_t *data, uint32_t size) {

	trace_mp_msg_send_enter(from, to, data, size);

	if (((uint32_t) data & 0x3) != 0) {
		return OPTIMSOC_MP_ERR_ALIGNMENT;
	}

	if (_optimsoc_mp_offload == MP_OFFLOAD_NAMP) {
		return OPTIMSOC_MP_ERR_GENERIC;
	} else {
		uint32_t addr, index;

		control_msg_alloc(to, &addr, &index);

		if (_optimsoc_mp_offload == MP_OFFLOAD_RMA) {
			return OPTIMSOC_MP_ERR_GENERIC;
		} else {
			control_msg_data(to, addr, data, size);
		}

		control_msg_final(to, size, index);
	}

	trace_mp_msg_send_leave();

	return OPTIMSOC_MP_SUCCESS;
}

/*

int optimsoc_mp_channel_connect(struct endpoint_handle *from,
                              struct endpoint_handle *to) {
    trace_chan_conn_begin(from, to);
    // currently needs to be called by the sender

    // Get credit from remote
    endpoint_channel_add_credit(from->ep, control_channel_connect(from, to));
    trace_chan_conn_end(from, to);
    return 0;
}

int optimsoc_mp_channel_recv(struct endpoint_handle *eph,
                           uint8_t* buffer, uint32_t buffer_size,
                           uint32_t *received_size) {
    int ret = 0;
    uint32_t read_ptr;

    while(endpoint_empty(eph->ep)) {
#ifdef RUNTIME
        optimsoc_thread_yield(optimsoc_thread_current());
#endif
    }

    endpoint_pop(eph->ep, &read_ptr);
    trace_ep_bufferstate(eph->ep, endpoint_channel_get_fillstate(eph->ep));

    *received_size = eph->ep->buffer->data_size[read_ptr];
    if (*received_size <= buffer_size) {
        memcpy(buffer, (void*)eph->ep->buffer->data[read_ptr], *received_size);
    } else {
        memcpy(buffer, (void*)eph->ep->buffer->data[read_ptr], buffer_size);
        *received_size = buffer_size;
        ret = OPTIMSOC_MP_ERROR_BUFFEROVERFLOW;
    }

    control_channel_sendcredit(eph, 1);

    return ret;
}

int optimsoc_mp_channel_pause(struct endpoint_handle *ep) {
    control_channel_sendcredit(ep, -1);
    return 0;
}

int optimsoc_mp_channel_continue(struct endpoint_handle *ep) {
    control_channel_sendcredit(ep, ep->ep->buffer->size-1);
    return 0;
}

int optimsoc_mp_channel_send(struct endpoint_handle *from,
                           struct endpoint_handle *to,
                           uint8_t* buffer,
                           uint32_t size) {
    //printf("%d\n", from->ep->remotecredit);
    trace_chan_send_begin(from, size);

    if (from->ep->remotecredit == 0) {
        if (endpoint_full(from->ep)) {
            assert(0==1);
            while (from->ep->remotecredit == 0) {}

            printf("TODO: Drain sender\n");
        }

        while (from->ep->remotecredit == 0) { }
    }

    uint32_t oldcredit;

    do {
        oldcredit = from->ep->remotecredit;
    } while (or1k_sync_cas((void*)&(from->ep->remotecredit), oldcredit, oldcredit-1) != oldcredit);

    trace_chan_send_xmit(from);

//    if (_optimsoc_mp_data == OPTIMSOC_MP_DATA_FIFO) {
        control_channel_send(to, buffer, size);
//    } else {
//        dma_transfer()
//        control_channel_send(to, buffer, size);
//    }

    trace_chan_send_end(from);

    return 0;
}

int optimsoc_mp_channel_send_i(struct endpoint_handle *from,
                             struct endpoint_handle *to,
                             uint8_t* buffer,
                             uint32_t size) {

    if (from->ep->remotecredit == 0) {
        printf("Insufficient credit in %p\n", from->ep);

        return -1;
    }

    control_channel_send(to, buffer, size);

    // TODO: CAS!
    from->ep->remotecredit--;

    return 0;
}*/


/*
int optimsoc_mp_msg_recv(optimsoc_mp_endpoint_handle eph, uint8_t *buffer,
                         uint32_t buffer_size, uint32_t *received_size) {
    int ret = 0;

    // TODO: This needs to be extended as another thread may picked up
    // the data from the endpoint
    while(endpoint_empty(eph->ep)) {
#ifdef RUNTIME
        optimsoc_thread_yield(optimsoc_thread_current());
#endif
    }

    endpoint_msg_recv(eph->ep, (uint32_t*) buffer, buffer_size, received_size);

    return ret;
}*/

void optimsoc_mp_trace_config_set(optimsoc_mp_trace_config_t config) {
	_mp_trace_config = config;
}

optimsoc_mp_trace_config_t optimsoc_mp_trace_config_get() {
	return _mp_trace_config;
}

