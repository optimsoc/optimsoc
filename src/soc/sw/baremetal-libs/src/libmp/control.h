#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <optimsoc-baremetal.h>

#include "endpoints.h"

#define NOC_CLASS_FIFO     0
#define NOC_CLASS_DMA      1
#define NOC_CLASS_MP       2

// The protocol for the MCAPI message passing uses control
// messages via the NoC.
struct {
    uint32_t buffer[16];
    volatile unsigned int done;
} ctrl_request;

#define CTRL_REQUEST_MSB (OPTIMSOC_SRC_LSB-1)
#define CTRL_REQUEST_LSB (CTRL_REQUEST_MSB-3)
#define CTRL_REQUEST_GETEP_REQ            0
#define CTRL_REQUEST_GETEP_RESP           1
#define CTRL_REQUEST_MSG_ALLOC_REQ        2
#define CTRL_REQUEST_MSG_ALLOC_RESP       3
#define CTRL_REQUEST_MSG_DATA             4
#define CTRL_REQUEST_MSG_COMPLETE         5
#define CTRL_REQUEST_CHAN_CONNECT_REQ     6
#define CTRL_REQUEST_CHAN_CONNECT_RESP    7
#define CTRL_REQUEST_CHAN_DATA            8
#define CTRL_REQUEST_CHAN_CREDIT          9
#define CTRL_REQUEST_10                   10
#define CTRL_REQUEST_11                   11
#define CTRL_REQUEST_12                   12
#define CTRL_REQUEST_13                   13
#define CTRL_REQUEST_14                   14
#define CTRL_REQUEST_15                   15
// HINT: if you add more, you have to adjust LSB!

#define CTRL_REQUEST_ACK 1
#define CTRL_REQUEST_NACK 0


void control_init();

struct endpoint *control_get_endpoint(uint32_t domain, uint32_t node,
                                      uint32_t port);
uint32_t control_msg_alloc(struct endpoint_handle *eph, uint32_t size);
void control_msg_data(struct endpoint_handle *ep, uint32_t address, void* buffer,
                      uint32_t size);

uint32_t control_channel_connect(struct endpoint_handle *from, struct endpoint_handle *to);
void control_channel_send(struct endpoint_handle *ep, uint8_t *data, uint32_t size);
void control_channel_sendcredit(struct endpoint_handle *ep, int32_t credit);

///////////////////////////////////////////////////////////////////////////////
//
// Tracing

#define TRACE_ENABLE_FUNCTIONS
//#define TRACE_ENABLE_LOWLEVEL

#define TRACE_EP_GET_BEGIN    0x300
#define TRACE_EP_GET_END      0x301
#define TRACE_MSG_ALLOC_BEGIN 0x302
#define TRACE_MSG_ALLOC_END   0x303
#define TRACE_MSG_DATA_BEGIN  0x304
#define TRACE_MSG_DATA_END    0x305
#define TRACE_CHAN_CONN_BEGIN 0x306
#define TRACE_CHAN_CONN_END   0x307
#define TRACE_CHAN_SEND_BEGIN 0x308
#define TRACE_CHAN_SEND_XMIT  0x309
#define TRACE_CHAN_SEND_END   0x30a

static inline void trace_ep_get_req_begin(uint32_t domain, uint32_t node,
                                          uint32_t port) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_EP_GET_BEGIN, domain);
    OPTIMSOC_TRACE(TRACE_EP_GET_BEGIN, node);
    OPTIMSOC_TRACE(TRACE_EP_GET_BEGIN, port);
#endif
}

static inline void trace_ep_get_req_end(struct endpoint *endpoint) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_EP_GET_END, endpoint);
#endif
}

static inline void trace_msg_alloc_begin(struct endpoint_handle* ep,
                                         uint32_t size) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_BEGIN, ep);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_BEGIN, size);
#endif
}

static inline void trace_msg_alloc_end(struct endpoint_handle* ep,
                                       uint32_t address) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_END, ep);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_END, address);
#endif
}

static inline void trace_msg_data_begin(struct endpoint_handle *eph,
                                        uint32_t address,
                                        uint32_t size) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_MSG_DATA_BEGIN, eph);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_BEGIN, address);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_BEGIN, size);
#endif
}

static inline void trace_msg_data_end(struct endpoint_handle *eph) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_MSG_DATA_END, eph);
#endif
}

static inline void trace_chan_conn_begin(struct endpoint_handle *from,
        struct endpoint_handle *to) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_CHAN_CONN_BEGIN, from);
    OPTIMSOC_TRACE(TRACE_CHAN_CONN_BEGIN, to);
#endif
}

static inline void trace_chan_conn_end(struct endpoint_handle *from,
        struct endpoint_handle *to) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_CHAN_CONN_END, from);
    OPTIMSOC_TRACE(TRACE_CHAN_CONN_END, to);
#endif
}

static inline void trace_chan_send_begin(struct endpoint_handle *eph,
                                        uint32_t size) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_CHAN_SEND_BEGIN, eph);
    OPTIMSOC_TRACE(TRACE_CHAN_SEND_BEGIN, size);
#endif
}

static inline void trace_chan_send_xmit(struct endpoint_handle *eph) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_CHAN_SEND_XMIT, eph);
#endif
}

static inline void trace_chan_send_end(struct endpoint_handle *eph) {
#ifdef TRACE_ENABLE_FUNCTIONS
    OPTIMSOC_TRACE(TRACE_CHAN_SEND_END, eph);
#endif
}

#define TRACE_EP_GET_REQ_SEND     0x340
#define TRACE_EP_GET_REQ_RECV     0x341
#define TRACE_EP_GET_RESP_SEND    0x342
#define TRACE_EP_GET_RESP_RECV    0x343
#define TRACE_MSG_ALLOC_REQ_SEND  0x344
#define TRACE_MSG_ALLOC_REQ_RECV  0x345
#define TRACE_MSG_ALLOC_RESP_SEND 0x346
#define TRACE_MSG_ALLOC_RESP_RECV 0x347
#define TRACE_MSG_DATA_SEND       0x348
#define TRACE_MSG_DATA_RECV       0x349
#define TRACE_MSG_COMPLETE_SEND   0x34a
#define TRACE_MSG_COMPLETE_RECV   0x34b

static inline void trace_ep_get_req_send(uint32_t domain, uint32_t node,
                                         uint32_t port) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_SEND, domain);
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_SEND, node);
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_SEND, port);
#endif
}

static inline void trace_ep_get_req_recv(uint32_t source, uint32_t node,
                                         uint32_t port) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_RECV, source);
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_RECV, node);
    OPTIMSOC_TRACE(TRACE_EP_GET_REQ_RECV, port);
#endif
}

static inline void trace_ep_get_resp_send(uint32_t source, struct endpoint *ep) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_EP_GET_RESP_SEND, source);
    OPTIMSOC_TRACE(TRACE_EP_GET_RESP_SEND, ep);
#endif
}

static inline void trace_ep_get_resp_recv(uint32_t domain, struct endpoint * ep) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_EP_GET_RESP_RECV, domain);
    OPTIMSOC_TRACE(TRACE_EP_GET_RESP_RECV, ep);
#endif
}

static inline void trace_msg_alloc_req_send(struct endpoint_handle* ep,
                                            uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_SEND, ep->domain);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_SEND, ep->node);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_SEND, ep->port);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_SEND, size);
#endif
}

static inline void trace_msg_alloc_req_recv(uint32_t source,
                                            struct endpoint* ep,
                                            uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_RECV, source);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_RECV, ep);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_REQ_RECV, size);
#endif
}

static inline void trace_msg_alloc_resp_send(uint32_t source,
                                             struct endpoint* ep,
                                             uint32_t address) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_RESP_SEND, source);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_RESP_SEND, ep);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_RESP_SEND, address);
#endif
}

static inline void trace_msg_alloc_resp_recv(struct endpoint_handle* eph,
                                             uint32_t address) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_RESP_RECV, eph);
    OPTIMSOC_TRACE(TRACE_MSG_ALLOC_RESP_RECV, address);
#endif
}

static inline void trace_msg_data_send(struct endpoint_handle *eph,
                                       uint32_t address,
                                       uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_DATA_SEND, eph);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_SEND, address);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_SEND, size);
#endif
}

static inline void trace_msg_data_recv(struct endpoint *ep,
                                       uint32_t address,
                                       uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_DATA_RECV, ep);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_RECV, address);
    OPTIMSOC_TRACE(TRACE_MSG_DATA_RECV, size);
#endif
}

static inline void trace_msg_complete_send(struct endpoint_handle *eph,
                                           uint32_t address,
                                           uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_SEND, eph);
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_SEND, address);
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_SEND, size);
#endif
}

static inline void trace_msg_complete_recv(struct endpoint *ep,
                                           uint32_t address,
                                           uint32_t size) {
#ifdef TRACE_ENABLE_LOWLEVEL
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_RECV, ep);
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_RECV, address);
    OPTIMSOC_TRACE(TRACE_MSG_COMPLETE_RECV, size);
#endif
}

#ifdef TRACE_ENABLE_FUNCTIONS
#undef TRACE_ENABLE_FUNCTIONS
#endif

#ifdef TRACE_ENABLE_LOWLEVEL
#undef TRACE_ENABLE_LOWLEVEL
#endif


#endif
