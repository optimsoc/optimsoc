#include "endpoints.h"
#include "control.h"

// The handles are opaque to the application
typedef struct endpoint_handle* optimsoc_mp_endpoint_handle;

#define __OPTIMSOC_INTERNAL__
#include "include/optimsoc-mp.h"

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
    OPTIMSOC_MP_CONTROL_FIFO = 0
} _optimsoc_mp_control;

enum {
    OPTIMSOC_MP_DATA_FIFO = 0,
    OPTIMSOC_MP_DATA_DMA = 1
} _optimsoc_mp_data;

int optimsoc_mp_initialize(struct optimsoc_mp_attributes *attr) {

    // Initialize endpoints
    endpoints_init();

    // Initialize control plane
    control_init();

    _optimsoc_mp_control = OPTIMSOC_MP_CONTROL_FIFO;
    _optimsoc_mp_data = OPTIMSOC_MP_DATA_FIFO;

    return 0;
}

char* optimsoc_mp_error_string(int errno) {
    char* str;

    switch(errno) {
    case OPTIMSOC_MP_ERROR_NOT_INITIALIZED:
        str = "not initialized";
        break;
    case OPTIMSOC_MP_ERROR_DOMAINS_NOT_SUPPORTED:
        str = "domains not supported";
        break;
    case OPTIMSOC_MP_ERROR_BUFFEROVERFLOW:
        str = "buffer overflow";
        break;
    default:
        str = "unknown error";
        break;
    }

    return str;
}

int optimsoc_mp_endpoint_create(struct endpoint_handle **eph,
                                uint32_t node, uint32_t port,
                                optimsoc_endpoint_type buffer_type,
                                uint32_t buffer_size, int overwrite_max_size) {

    // Endpoint creation only requires access to the local database, no
    // messages between the tiles
    *eph = endpoint_create(node, port, buffer_type, buffer_size, overwrite_max_size);

    return 0;
}

int optimsoc_mp_endpoint_get(struct endpoint_handle **eph, uint32_t tile,
                             uint32_t node, uint32_t port) {
    // This gets the endpoint from the local database or creates a request to
    // the tile if it is not found
    *eph = endpoint_get(tile, node, port);
    return 0;
}

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

    if (_optimsoc_mp_data == OPTIMSOC_MP_DATA_FIFO) {
        control_channel_send(to, buffer, size);
    } else {
//        dma_transfer()
//        control_channel_send(to, buffer, size);
    }

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
}

int optimsoc_mp_msg_send(optimsoc_mp_endpoint_handle from,
                         optimsoc_mp_endpoint_handle to, uint8_t *data,
                         uint32_t size) {

    uint32_t addr = control_msg_alloc(to, size);

    control_msg_data(to, addr, data, size);

    return 0;
}

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
}
