#include "endpoints.h"
#include "control.h"

// The handles are opaque to the application
typedef struct endpoint_handle* optimsoc_mp_endpoint_handle;

#define __OPTIMSOC_INTERNAL__
#include "include/optimsoc-mp.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*
 * All message passing functions are entered here from the software. The general
 * handling of the protocol is implemented here, but all accesses to the
 * endpoints need to be done via endpoint_* and all control messages need to be
 * send via control_*.
 */

int optimsoc_mp_initialize(struct optimsoc_mp_attributes *attr) {

    endpoints_init();

    control_init();

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

    *eph = endpoint_create(node, port, buffer_type, buffer_size, overwrite_max_size);

    return 0;
}

int optimsoc_mp_endpoint_get(struct endpoint_handle **eph, uint32_t tile,
                             uint32_t node, uint32_t port) {
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

int optimsoc_mp_channel_recv(struct endpoint_handle *ep,
                           uint8_t* buffer, uint32_t buffer_size,
                           uint32_t *received_size) {

    while(endpoint_empty(ep->ep)) { }

    // TODO: fill buffer
    ep->ep->buffer->read_ptr = _endpoint_addptrwrap(ep->ep, ep->ep->buffer->read_ptr, 1);
    trace_ep_bufferstate(ep->ep, endpoint_channel_get_fillstate(ep->ep));

    control_channel_sendcredit(ep, 1);
    return 0;
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

    if (from->ep->remotecredit == 0) {
        if (endpoint_full(from->ep)) {
            while (from->ep->remotecredit == 0) {}

            printf("TODO: Drain sender\n");
        }

        while (from->ep->remotecredit == 0) {}
    }

    control_channel_send(to, buffer, size);

    // TODO: CAS!
    from->ep->remotecredit--;

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
