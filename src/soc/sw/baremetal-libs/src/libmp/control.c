#include "control.h"
#include "endpoints.h"

#include <optimsoc-baremetal.h>
#include "../include/optimsoc-baremetal.h"

#ifdef RUNTIME
#include <optimsoc-runtime.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

unsigned int timeout_insns = 1000;

#define EXTRACT(x,msb,lsb) ((x>>lsb) & ~(~0 << (msb-lsb+1)))
#define SET(x,v,msb,lsb) (((~0 << (msb+1) | ~(~0 << lsb))&x) | ((v & ~(~0<<(msb-lsb+1))) << lsb))

void control_msg_handler(uint32_t* buffer,size_t len);

void control_init() {
    // Add handler so that received message are treated correctly
    // Class 1: control messages
    optimsoc_mp_simple_init();
    optimsoc_mp_simple_addhandler(NOC_CLASS_MP, &control_msg_handler);

    optimsoc_mp_simple_enable(0);
    optimsoc_mp_simple_enable(1);
    or1k_interrupts_enable();
}

// The following handler is called by the message interrupt service routine
void control_msg_handler(uint32_t* buffer,size_t len) {
    // Extract sender information
    unsigned int src = EXTRACT(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
    // Extract request type
    int req = EXTRACT(buffer[0],CTRL_REQUEST_MSB,CTRL_REQUEST_LSB);

    // Reply buffer
    uint32_t rbuffer[5];

    // Handle the respective request
    switch (req) {
    case CTRL_REQUEST_GETEP_REQ:
    {
        trace_ep_get_req_recv(src, buffer[1], buffer[2]);

        // This is the request to get an endpoint handle
        // Flit 1: node number
        // Flit 2: port number

        // Return the get endpoint response to sender
        rbuffer[0] = (src << OPTIMSOC_DEST_LSB) |
                     (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                     (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                     (CTRL_REQUEST_GETEP_RESP << CTRL_REQUEST_LSB);

        // Get endpoint handle for <thisdomain,node,port> where
        // this domain is the tile id
        struct endpoint_handle *eph = endpoint_get(optimsoc_get_tileid(),
                                                   buffer[1], buffer[2]);

        // If valid numbers and endpoint handle found
        if (
                //buffer[1] < MCA_MAX_NODES &&
                //buffer[2] < MCAPI_MAX_ENDPOINTS &&
                (eph!=0)) {
            // Return endpoint
            rbuffer[1] = (unsigned int) eph->ep;
        } else {
            // Signal this is an invalid endpoint
            rbuffer[1] = (int) -1;
        }

        trace_ep_get_resp_send(src, (struct endpoint*) rbuffer[1]);
        optimsoc_mp_simple_send(1,2,rbuffer);
        break;
    }
    case CTRL_REQUEST_MSG_ALLOC_REQ:
    {
        rbuffer[0] = (src << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_ALLOC_RESP << CTRL_REQUEST_LSB);

        struct endpoint *ep = (struct endpoint*) buffer[1];
        unsigned int size = buffer[2];

        trace_msg_alloc_req_recv(src, ep, size);

        uint32_t ptr;
        int rv = endpoint_alloc(ep, size, &ptr);
        if (rv == 0) {
            rbuffer[1] = CTRL_REQUEST_ACK;
            rbuffer[2] = ptr;
            trace_msg_alloc_resp_send(src, ep, ptr);
            optimsoc_mp_simple_send(1,3, rbuffer);

        } else {
            rbuffer[1] = CTRL_REQUEST_NACK;
            trace_msg_alloc_resp_send(src, ep, -1);
            optimsoc_mp_simple_send(1,2,rbuffer);
        }

        break;
    }
    case CTRL_REQUEST_MSG_DATA:
    {
        struct endpoint *ep = (struct endpoint*) buffer[1];

        endpoint_write(ep, buffer[2], buffer[3], (uint32_t*) &buffer[4], len-4);

        break;
    }
    case CTRL_REQUEST_MSG_COMPLETE:
    {
        struct endpoint *ep = (struct endpoint*) buffer[1];

        endpoint_write_complete(ep, buffer[2], buffer[3]);

#ifdef RUNTIME
        // TODO
        /*        if (ep->waiting_thread) {
                optimsoc_thread_resume(ep->waiting_thread);
                ep->waiting_thread = 0;
            }*/
#endif
        break;
    }
    case CTRL_REQUEST_CHAN_CONNECT_REQ:
    {
        struct endpoint *ep = (struct endpoint *) buffer[1];
        ep->remotedomain = (uint32_t) buffer[2];
        ep->remote = (struct endpoint *) buffer[3];

        rbuffer[0] = (src << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_CHAN_CONNECT_RESP << CTRL_REQUEST_LSB);

        rbuffer[1] = endpoint_channel_get_credit(ep);
        optimsoc_mp_simple_send(1, 2, rbuffer);
        break;
    }
    case CTRL_REQUEST_CHAN_DATA:
    {
        struct endpoint *ep = (struct endpoint *) buffer[1];
        uint32_t offset = buffer[2];
        uint32_t eom = buffer[3];

        endpoint_write(ep, ep->buffer->write_ptr, offset, (uint32_t*) &buffer[4], len-4);

        if (eom) {
            ep->buffer->data_size[ep->buffer->write_ptr] = offset*4 + eom;
            uint32_t ptr; // Move the pointer
            endpoint_push(ep, &ptr);
            trace_ep_bufferstate(ep, endpoint_channel_get_fillstate(ep));
        }


        break;
    }
    case CTRL_REQUEST_CHAN_CREDIT:
    {
      struct endpoint *ep = (struct endpoint *) buffer[1];
        uint32_t credit = buffer[2];

        if (credit == 0) {
            ep->remotecredit = 0;
        } else {
            ep->remotecredit += credit;
        }

        break;
    }
    case CTRL_REQUEST_GETEP_RESP:
    case CTRL_REQUEST_MSG_ALLOC_RESP:
    case CTRL_REQUEST_CHAN_CONNECT_RESP:
        // Forward the responses to the handler
        ctrl_request.buffer[0] = buffer[0];
        ctrl_request.buffer[1] = buffer[1];
        ctrl_request.buffer[2] = buffer[2];
        ctrl_request.buffer[3] = buffer[3];
        ctrl_request.buffer[4] = buffer[4];
        ctrl_request.done = 1;
        break;
    default:
        printf("Unknown request: %d\n",req);
        break;
    }
}

void control_wait_response() {
    // Get current interrupts by disabling temporarily
    uint32_t restore = or1k_interrupts_disable();
    // Enable all interrupts
    or1k_interrupts_enable();
    // Wait until the reply arrived
    while (ctrl_request.done == 0) { }
    // Restore previous state of interrupts
    or1k_interrupts_restore(restore);
}

struct endpoint *control_get_endpoint(uint32_t domain, uint32_t node,
                                      uint32_t port) {
    struct endpoint *ep;

    while (!optimsoc_mp_simple_ctready(domain, 0));

    trace_ep_get_req_begin(domain, node, port);

    do {
        // Try to retrieve from remote
        // We do this as long as we do not get a valid handle back (-1)
        ctrl_request.buffer[0] = (domain << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_GETEP_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = node;
        ctrl_request.buffer[2] = port;
        ctrl_request.done = 0;

        trace_ep_get_req_send(domain, node, port);

        optimsoc_mp_simple_send(0,3,ctrl_request.buffer);

        control_wait_response();

        ep = (struct endpoint*) ctrl_request.buffer[1];
        trace_ep_get_resp_recv(domain, ep);

        if ((int)ep==-1) {
#ifdef RUNTIME
            assert(0);
            // TODO: Reactivate
            //optimsoc_thread_yield();
#endif
            for (int t=0;t<timeout_insns;t++) { asm __volatile__("l.nop 0x0"); }
            timeout_insns = timeout_insns * 10; // somewhat arbitrary..
        }
    } while ((int)ep==-1);

    trace_ep_get_req_end(ep);

    return ep;
}

uint32_t control_msg_alloc(struct endpoint_handle *to_ep, uint32_t size) {
    trace_msg_alloc_begin(to_ep, size);

    // Wait until receive_endpoint is ready to receive (allocate)
    do {
        // Try to retrieve from remote
        // We do this as long as we do not get a valid handle back (-1)
        ctrl_request.buffer[0] = (to_ep->domain << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_ALLOC_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) to_ep->ep;
        ctrl_request.buffer[2] = (unsigned int) size;
        ctrl_request.done = 0;

        trace_msg_alloc_req_send(to_ep, size);

        optimsoc_mp_simple_send(0,3,ctrl_request.buffer);

        control_wait_response();

        if (ctrl_request.buffer[1]==CTRL_REQUEST_NACK) {
            for (int t=0;t<timeout_insns;t++) { asm __volatile__("l.nop 0x0"); }
            timeout_insns = timeout_insns * 10; // somewhat arbitrary..
        }

    } while (ctrl_request.buffer[1]==CTRL_REQUEST_NACK);

    trace_msg_alloc_end(to_ep, ctrl_request.buffer[2]);

    return ctrl_request.buffer[2];
}

void control_msg_data(struct endpoint_handle *ep, uint32_t address, void* buffer,
                      uint32_t size) {
    // TODO: what if size%4!=0?
    assert(size % 4 == 0);

    trace_msg_data_begin(ep, address, size);

    unsigned int words = (size+3)>>2;
    unsigned int wordsperpacket = optimsoc_noc_maxpacketsize()-4;

    for (int i=0;i<words;i=i+wordsperpacket) {
        ctrl_request.buffer[0] = (ep->domain << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_DATA << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) ep->ep;
        ctrl_request.buffer[2] = (uint32_t) address;
        ctrl_request.buffer[3] = i;

        int sz = words - i;
        if (sz>wordsperpacket)
            sz = wordsperpacket;

        for (int d=0;d<sz;d++) {
            ctrl_request.buffer[4+d] = ((unsigned int *)buffer)[i+d];
        }

        trace_msg_data_send(ep, ctrl_request.buffer[2], sz);
        optimsoc_mp_simple_send(0, 4+sz,ctrl_request.buffer);
    }

    ctrl_request.buffer[0] = (ep->domain << OPTIMSOC_DEST_LSB) |
            (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
            (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
            (CTRL_REQUEST_MSG_COMPLETE << CTRL_REQUEST_LSB);
    ctrl_request.buffer[1] = (unsigned int) ep->ep;
    ctrl_request.buffer[2] = (uint32_t) address;
    ctrl_request.buffer[3] = size;

    trace_msg_complete_send(ep, address, size);
    optimsoc_mp_simple_send(0, 4, ctrl_request.buffer);

    trace_msg_data_end(ep);
}

uint32_t control_channel_connect(struct endpoint_handle *from,
                                 struct endpoint_handle *to) {
    ctrl_request.buffer[0] = (to->domain << OPTIMSOC_DEST_LSB) |
            (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
            (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
            (CTRL_REQUEST_CHAN_CONNECT_REQ << CTRL_REQUEST_LSB);
    ctrl_request.buffer[1] = (unsigned int) to->ep;
    ctrl_request.buffer[2] = (unsigned int) from->domain;
    ctrl_request.buffer[3] = (unsigned int) from->ep;

    ctrl_request.done = 0;
    optimsoc_mp_simple_send(0, 4, ctrl_request.buffer);

    control_wait_response();

    return ctrl_request.buffer[1];
}

void control_channel_send(struct endpoint_handle *ep, uint8_t *data, uint32_t size) {
    unsigned int words = (size+3)>>2;
    unsigned int wordsperpacket = optimsoc_noc_maxpacketsize()-4;

    for (int i=0;i<words;i=i+wordsperpacket) {
        ctrl_request.buffer[0] = (ep->domain << OPTIMSOC_DEST_LSB) |
                (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_CHAN_DATA << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) ep->ep;
        ctrl_request.buffer[2] = i;
        if (((i+wordsperpacket) >= words)) {
            ctrl_request.buffer[3] = size - i * wordsperpacket;
        } else {
            ctrl_request.buffer[3] = 0;
        }

        int sz = words - i;
        if (sz>wordsperpacket)
            sz = wordsperpacket;

        for (int d=0;d<sz;d++) {
            ctrl_request.buffer[4+d] = ((unsigned int *)data)[i+d];
        }

        optimsoc_mp_simple_send(0,4+sz,ctrl_request.buffer);
    }
}

void control_channel_sendcredit(struct endpoint_handle *ep, int32_t credit) {
    ctrl_request.buffer[0] = (ep->ep->remotedomain << OPTIMSOC_DEST_LSB) |
            (NOC_CLASS_MP << OPTIMSOC_CLASS_LSB) |
            (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
            (CTRL_REQUEST_CHAN_CREDIT << CTRL_REQUEST_LSB);
    ctrl_request.buffer[1] = (unsigned int) ep->ep->remote;
    ctrl_request.buffer[2] = credit;

    optimsoc_mp_simple_send(0,3, ctrl_request.buffer);
}
