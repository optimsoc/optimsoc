#ifndef ENDPOINTS_H_
#define ENDPOINTS_H_

#include <optimsoc-baremetal.h>

#ifdef RUNTIME
#include <optimsoc-runtime.h>
#endif

// In byte
#define MAX_MESSAGE_SIZE 64
#define MAX_PACKET_SIZE  64

#define MAX_ENDPOINTS 100

// The endpoint handle contains the metainformation of an endpoint
// and a pointer to the actual endpoint if local
struct endpoint_handle {
    enum { LOCAL, REMOTE } type;
    struct endpoint  *ep;
    unsigned int   domain;
    unsigned int   node;
    unsigned int   port;
};

typedef enum { MESSAGE = 0, CHANNEL = 1 } endpoint_buffer_type;

struct endpoint_buffer {
    volatile uint32_t     **data;
    volatile uint32_t     *data_size;
    endpoint_buffer_type  type;
    volatile unsigned int size;
    volatile unsigned int read_ptr;
    volatile unsigned int write_ptr;
};

// The endpoint data structure holds the actual buffer and the flags
struct endpoint {
    struct endpoint_buffer *buffer;
    volatile unsigned int  flags;
    uint32_t remotedomain;
    struct endpoint   *remote;
    volatile uint32_t remotecredit;
#ifdef RUNTIME
    volatile optimsoc_thread_t waiting_thread;
#endif
};

#define EP_FLAG_READY       1
#define EP_FLAG_ALLOC       2
#define EP_FLAG_MSG         4
#define EP_FLAG_CONNECTED   8
#define EP_FLAG_DIRECTION   16
#define EP_FLAG_DIRECTION_SEND 0
#define EP_FLAG_DIRECTION_RECV 1
#define EP_FLAG_CHANNELTYPE 32
#define EP_FLAG_CHANNELTYPE_PACKET 0
#define EP_FLAG_CHANNELTYPE_SCALAR 1
#define EP_FLAG_OPENED      64

void endpoints_init();

struct endpoint_handle *endpoint_create(uint32_t node, uint32_t port,
                                        endpoint_buffer_type buffer_type,
                                        uint32_t buffer_size,
                                        int overwrite_max_size);

int endpoint_add(struct endpoint_handle *ep);
struct endpoint_handle *endpoint_get(unsigned int domain, unsigned int node,
                                     unsigned int port);
struct endpoint_handle *endpoint_verify(struct endpoint_handle *eph);
void endpoint_delete(struct endpoint_handle *eph);
int endpoint_generate_portnum(unsigned int *port);

unsigned int endpoints_localnum();

int endpoint_alloc(struct endpoint *ep, uint32_t size, uint32_t *ptr);
void endpoint_write(struct endpoint *ep, uint32_t ptr, uint32_t offset,
                    uint32_t *buffer, uint32_t size);
void endpoint_write_complete(struct endpoint *ep, uint32_t ptr, uint32_t size);

int endpoint_empty(struct endpoint *ep);
int endpoint_full(struct endpoint *ep);

int endpoint_pop(struct endpoint *ep, uint32_t *ptr);
int endpoint_push(struct endpoint *ep, uint32_t *ptr);

int endpoint_msg_available(struct endpoint *ep);
void endpoint_msg_recv(struct endpoint *ep, uint32_t *buffer,
                       uint32_t buffer_size, uint32_t *received);

uint32_t endpoint_channel_get_credit(struct endpoint *ep);
uint32_t endpoint_channel_get_fillstate(struct endpoint *ep);

/**
 * Add credit
 */
void endpoint_channel_add_credit(struct endpoint *ep, uint32_t credit);


///////////////////////////////////////////////////////////////////////////////
//
// Tracing

#define TRACE_ENABLE

#define TRACE_EP_CREATE      0x380
#define TRACE_EP_BUFFERSTATE 0x381

static inline void trace_ep_create(struct endpoint *ep) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(TRACE_EP_CREATE, ep);
#endif
}

static inline void trace_ep_bufferstate(struct endpoint *ep,
                                        uint32_t fill_state) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(TRACE_EP_BUFFERSTATE, ep);
    OPTIMSOC_TRACE(TRACE_EP_BUFFERSTATE, fill_state);
#endif
}

#ifdef TRACE_ENABLE
#undef TRACE_ENABLE
#endif

#endif /* ENDPOINTS_H_ */
