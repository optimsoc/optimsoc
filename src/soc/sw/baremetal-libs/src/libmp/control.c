#include <optimsoc-baremetal.h>
#include "control.h"
#include "endpoint.h"
#include "mgmt.h"
#include "buffer.h"
#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define NOC_CLASS_FIFO     0
#define NOC_CLASS_DMA      1
#define NOC_CLASS_MP       2

// The protocol for the MCAPI message passing uses control
// messages via the NoC.
struct {
    uint32_t buffer[16];
    volatile unsigned int done;
} ctrl_request;

#define MP_TYPE_MSB (OPTIMSOC_SRC_LSB-1)
#define MP_TYPE_LSB (MP_TYPE_MSB-3)
#define MP_TYPE_GETEP_REQ            0
#define MP_TYPE_GETEP_RESP           1
#define MP_TYPE_ALLOC_REQ            2
#define MP_TYPE_ALLOC_RESP           3
#define MP_TYPE_FINAL                4
#define MP_TYPE_DATA                15

#define MP_REQID_MSB (MP_TYPE_LSB-1)
#define MP_REQID_LSB (MP_REQID_MSB-9)

#define MP_ALLOCRESP_FLAG_MSB (MP_REQID_LSB-1)
#define MP_ALLOCRESP_FLAG_LSB (MP_ALLOCRESP_FLAG_MSB-1)
#define MP_ALLOCRESP_FLAG_ACK 0
#define MP_ALLOCRESP_FLAG_NACK 1
#define MP_ALLOCRESP_FLAG_NACK_RELOC 3

static unsigned int timeout_insns = 1000;

#define EXTRACT(x,msb,lsb) extract_bits(x, msb, lsb)
#define SET(x,v,msb,lsb) set_bits(x, v, msb, lsb)
#define SET_HEADER_FIELD(h, t, v) set_bits(h, v, OPTIMSOC_##t##_MSB, OPTIMSOC_##t##_LSB)
#define SET_MPHEADER_FIELD(h, t, v) set_bits(h, v, MP_##t##_MSB, MP_##t##_LSB)

void control_msg_handler(uint32_t* buffer, size_t len);

void control_init() {
    // Add handler so that received message are treated correctly
    optimsoc_mp_simple_init();
    optimsoc_mp_simple_addhandler(NOC_CLASS_FIFO, &control_msg_handler);

    optimsoc_mp_simple_enable(0);
    optimsoc_mp_simple_enable(1);
    or1k_interrupts_enable();
}


// The following handler is called by the message interrupt service routine
void control_msg_handler(uint32_t* buffer, size_t len) {
    // Extract sender information
	uint32_t src = EXTRACT(buffer[0], OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    // Extract request type
	uint32_t req = EXTRACT(buffer[0], MP_TYPE_MSB, MP_TYPE_LSB);
    uint32_t reqid = EXTRACT(buffer[0], MP_REQID_MSB, MP_REQID_LSB);

    trace_mp_ll_ctrl_msghandler_enter(src, req, reqid);

    // Reply buffer
    uint32_t rbuffer[5];

    // Some variables used in the switch, need to define outside..
    optimsoc_mp_result_t ret;
    mp_cbuffer_result_t bret;

    // Handle the respective request
    switch (req) {
    case MP_TYPE_GETEP_REQ:
    {
        trace_proto_getep_req_recv(src, buffer[1], buffer[2], buffer[3]);

        rbuffer[0] = 0;
    	SET_HEADER_FIELD(&rbuffer[0], DEST, src);
    	SET_HEADER_FIELD(&rbuffer[0], CLASS, NOC_CLASS_FIFO);
    	SET_HEADER_FIELD(&rbuffer[0], SRC, optimsoc_get_tileid());
    	SET_MPHEADER_FIELD(&rbuffer[0], TYPE, MP_TYPE_GETEP_RESP);
    	SET_MPHEADER_FIELD(&rbuffer[0], REQID, reqid);

    	struct endpoint_handle *eph;
    	ret = mgmt_get(&eph, buffer[1], buffer[2], buffer[3], 1);

        // If valid numbers and endpoint handle found
        if ((ret == OPTIMSOC_MP_SUCCESS) && (eph->type == LOCAL)) {
            // Return endpoint
            rbuffer[1] = (uint32_t) eph->ptr.local;
        } else {
            // Signal this is an invalid endpoint
            rbuffer[1] = 0xffffffff;
        }

        trace_proto_getep_resp_send(src, rbuffer[1]);

        optimsoc_mp_simple_send(1, 2, rbuffer);

        break;
    }
    case MP_TYPE_ALLOC_REQ:
    {
    	rbuffer[0] = 0;
    	SET_HEADER_FIELD(&rbuffer[0], DEST, src);
    	SET_HEADER_FIELD(&rbuffer[0], CLASS, NOC_CLASS_FIFO);
    	SET_HEADER_FIELD(&rbuffer[0], SRC, optimsoc_get_tileid());
    	SET_MPHEADER_FIELD(&rbuffer[0], TYPE, MP_TYPE_ALLOC_RESP);
    	SET_MPHEADER_FIELD(&rbuffer[0], REQID, reqid);

        struct endpoint_local *ep = (struct endpoint_local*) buffer[1];

        trace_proto_alloc_req_recv(src, ep);

        bret = mp_cbuffer_reserve(ep->recv_buffer, &rbuffer[2]);
        if (bret != MP_CBUFFER_SUCCESS) {
        	SET_MPHEADER_FIELD(&rbuffer[0], ALLOCRESP_FLAG, MP_ALLOCRESP_FLAG_NACK);

        	trace_proto_alloc_resp_send(MP_ALLOCRESP_FLAG_NACK, 0, 0);

            optimsoc_mp_simple_send(1, 1, rbuffer);
        } else {
        	SET_MPHEADER_FIELD(&rbuffer[0], ALLOCRESP_FLAG, MP_ALLOCRESP_FLAG_ACK);

        	uint32_t addr = (uint32_t) ep->recv_buffer->data;
        	addr += rbuffer[2] * (1<<ep->recv_buffer->max_msg_size);
        	rbuffer[1] = addr;

        	trace_proto_alloc_resp_send(MP_ALLOCRESP_FLAG_ACK, addr, rbuffer[2]);

        	optimsoc_mp_simple_send(1, 3, rbuffer);
        }

        break;
    }
    case MP_TYPE_DATA:
    {
        uint32_t* addr = (uint32_t*) buffer[1];
        uint32_t size = len - 2;

        trace_proto_data_recv(src, addr, size);

        memcpy(addr, &buffer[2], size*4);

        break;
    }
    case MP_TYPE_FINAL:
    {
        struct endpoint_local *ep = (struct endpoint_local*) buffer[1];
        uint32_t idx = buffer[2];
        uint32_t size = buffer[3];

        trace_proto_final_recv(src, ep, idx, size);

        mp_cbuffer_commit(ep->recv_buffer, idx, size);

        break;
    }
    case MP_TYPE_GETEP_RESP:
    case MP_TYPE_ALLOC_RESP:
        // Forward the responses to the handler
        ctrl_request.buffer[0] = buffer[0];
        ctrl_request.buffer[1] = buffer[1];
        ctrl_request.buffer[2] = buffer[2];
        ctrl_request.buffer[3] = buffer[3];
        ctrl_request.buffer[4] = buffer[4];
        ctrl_request.done = 1;
        break;
    default:
        printf("Unknown request: %lu\n", req);
        break;
    }

    trace_mp_ll_ctrl_msghandler_leave();
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

optimsoc_mp_result_t control_get_endpoint(struct endpoint_remote ** ep, uint32_t tile,
		uint32_t domain, uint32_t node, uint32_t port) {

    trace_mp_ll_ctrl_getep_enter(tile, domain, node, port);

    *ep = calloc(1, sizeof(struct endpoint_remote));
    if (!*ep) {
    	return OPTIMSOC_MP_ERR_NO_MEMORY;
    }

    (*ep)->tile = tile;

    while (!optimsoc_mp_simple_ctready(tile, 0));

    timeout_insns = 1000;

    do {
        // Try to retrieve from remote
        // We do this as long as we do not get a valid handle back (-1)
    	ctrl_request.buffer[0] = 0;
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], DEST, tile);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], CLASS, NOC_CLASS_FIFO);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], SRC, optimsoc_get_tileid());
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], TYPE, MP_TYPE_GETEP_REQ);
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], REQID, 0);

        ctrl_request.buffer[1] = domain;
        ctrl_request.buffer[2] = node;
        ctrl_request.buffer[3] = port;
        ctrl_request.done = 0;

        trace_proto_getep_req_send(tile, domain, node, port);

        optimsoc_mp_simple_send(0, 4, ctrl_request.buffer);

        control_wait_response();

        (*ep)->addr = ctrl_request.buffer[1];
        trace_proto_getep_resp_recv((*ep)->addr);

        if ((*ep)->addr == 0xffffffff) {
            for (int t=0; t<timeout_insns; t++) { asm __volatile__("l.nop 0x0"); }
        }
    } while ((*ep)->addr == 0xffffffff);

    trace_mp_ll_ctrl_getep_leave(*ep, (*ep)->addr);

    return OPTIMSOC_MP_SUCCESS;
}

optimsoc_mp_result_t control_msg_alloc(struct endpoint_handle *to_ep,
		uint32_t *addr, uint32_t *index) {
	trace_mp_ll_ctrl_alloc_enter(to_ep);

	if ((to_ep->type != REMOTE) ||
			(to_ep->ptr.remote->tile == optimsoc_get_tileid())) {
		return OPTIMSOC_MP_ERR_INVALID_PARAMETER;
	}

	uint32_t alloc_flag;

    // Wait until receive_endpoint is ready to receive (allocate)
    do {
        // Try to retrieve from remote
        // We do this as long as we do not get a valid handle back (-1)
    	ctrl_request.buffer[0] = 0;
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], DEST, to_ep->ptr.remote->tile);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], CLASS, NOC_CLASS_FIFO);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], SRC, optimsoc_get_tileid());
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], TYPE, MP_TYPE_ALLOC_REQ);
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], REQID, 0);

        ctrl_request.buffer[1] = (unsigned int) to_ep->ptr.remote->addr;
        ctrl_request.done = 0;

        trace_proto_alloc_req_send(to_ep->ptr.remote->tile, to_ep->ptr.remote->addr);

        optimsoc_mp_simple_send(0, 2, ctrl_request.buffer);

        control_wait_response();

        alloc_flag = EXTRACT(ctrl_request.buffer[0], MP_ALLOCRESP_FLAG_MSB, MP_ALLOCRESP_FLAG_LSB);

        trace_proto_alloc_resp_recv(alloc_flag, ctrl_request.buffer[1],
        		ctrl_request.buffer[2]);

        if (alloc_flag != MP_ALLOCRESP_FLAG_ACK) {
            for (int t=0;t<timeout_insns;t++) { asm __volatile__("l.nop 0x0"); }
        }

    } while (alloc_flag != MP_ALLOCRESP_FLAG_ACK);

    *addr = ctrl_request.buffer[1];
    *index = ctrl_request.buffer[2];

    trace_mp_ll_ctrl_alloc_leave(*addr, *index);

    return OPTIMSOC_MP_SUCCESS;
}

void control_msg_data(struct endpoint_handle *ep, uint32_t address,
		uint32_t* buffer, uint32_t size) {
    trace_mp_ll_ctrl_data_enter(ep, address, buffer, size);

    uint32_t addr = address;

    uint32_t words = (size+3)>>2;
    uint32_t wordsperpacket = optimsoc_noc_maxpacketsize()-2;

    for (int i=0;i<words;i=i+wordsperpacket) {
    	ctrl_request.buffer[0] = 0;
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], DEST, ep->ptr.remote->tile);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], CLASS, NOC_CLASS_FIFO);
    	SET_HEADER_FIELD(&ctrl_request.buffer[0], SRC, optimsoc_get_tileid());
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], TYPE, MP_TYPE_DATA);
    	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], REQID, 0);
        ctrl_request.buffer[1] = (uint32_t) addr;

        int sz = words - i;
        if (sz>wordsperpacket)
            sz = wordsperpacket;

        memcpy(&ctrl_request.buffer[2], &buffer[i], 4*sz);

        trace_proto_data_send(ep, addr, sz);

        optimsoc_mp_simple_send(0, 2+sz, ctrl_request.buffer);

        addr += sz*4;
    }

    trace_mp_ll_ctrl_data_leave();
}

void control_msg_final(struct endpoint_handle *ep, uint32_t index, uint32_t size) {
	trace_mp_ll_ctrl_final_enter(ep, index, size);

	ctrl_request.buffer[0] = 0;
	SET_HEADER_FIELD(&ctrl_request.buffer[0], DEST, ep->ptr.remote->tile);
	SET_HEADER_FIELD(&ctrl_request.buffer[0], CLASS, NOC_CLASS_FIFO);
	SET_HEADER_FIELD(&ctrl_request.buffer[0], SRC, optimsoc_get_tileid());
	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], TYPE, MP_TYPE_FINAL);
	SET_MPHEADER_FIELD(&ctrl_request.buffer[0], REQID, 0);

	ctrl_request.buffer[1] = ep->ptr.remote->addr;
	ctrl_request.buffer[2] = index;
	ctrl_request.buffer[3] = size;

    optimsoc_mp_simple_send(0, 4, ctrl_request.buffer);

	trace_mp_ll_ctrl_final_leave();
}

/*uint32_t control_channel_connect(struct endpoint_handle *from,
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
}*/
