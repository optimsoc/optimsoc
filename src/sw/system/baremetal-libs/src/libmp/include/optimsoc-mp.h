#ifndef __OPTIMSOC_MP_H__
#define __OPTIMSOC_MP_H__

#include <stdint.h>

#ifndef __OPTIMSOC_INTERNAL__
typedef uint32_t optimsoc_mp_endpoint_handle;
#endif

typedef enum { OPTIMSOC_MP_EP_CONNECTIONLESS = 0,
               OPTIMSOC_MP_EP_CHANNEL = 1 } optimsoc_endpoint_type;

#define OPTIMSOC_MP_CONF_DOMAINS 1

struct optimsoc_mp_attributes {
    uint32_t flags;
};

int optimsoc_mp_initialize(struct optimsoc_mp_attributes*);
int optimsoc_mp_is_initialized();

/**
 * Create an endpoint
 *
 * Creates a message passing endpoint in this tile for a specific node on a
 * port. Other endpoints can send messages or connect to the endpoint via the
 * <tile, node, port> tuple.
 *
 * The buffer can either be connection-less or connection-oriented. When
 * connection-oriented you need to connect two endpoints to a channel first.
 *
 * The buffer_size is the number of messages or packets that can be used. If
 * messages or packets are larger than the default size, the maximum element
 * size can be overwritten for this endpoint.
 *
 * @return 0 if success, error code otherwise
 */

int optimsoc_mp_endpoint_create(optimsoc_mp_endpoint_handle *endpoint,
                                uint32_t node, uint32_t port,
                                optimsoc_endpoint_type buffer_type,
                                uint32_t buffer_size, int overwrite_max_size);

int optimsoc_mp_endpoint_get(optimsoc_mp_endpoint_handle *endpoint,
                             uint32_t tile, uint32_t node, uint32_t port);

int optimsoc_mp_channel_connect(optimsoc_mp_endpoint_handle from,
                             optimsoc_mp_endpoint_handle to);

int optimsoc_mp_channel_send(optimsoc_mp_endpoint_handle from,
                           optimsoc_mp_endpoint_handle to,
                           uint8_t* buffer, uint32_t size);

int optimsoc_mp_channel_send_i(optimsoc_mp_endpoint_handle from,
                               optimsoc_mp_endpoint_handle to,
                               uint8_t* buffer, uint32_t size);

int optimsoc_mp_channel_recv(optimsoc_mp_endpoint_handle ep,
                           uint8_t* buffer, uint32_t buffer_size,
                           uint32_t *received_size);

// Channel migration
int optimsoc_mp_channel_pause(optimsoc_mp_endpoint_handle ep);

int optimsoc_mp_channel_continue(optimsoc_mp_endpoint_handle ep);

#define OPTIMSOC_MP_ERROR_NOT_INITIALIZED       -1
#define OPTIMSOC_MP_ERROR_DOMAINS_NOT_SUPPORTED -2

char* optimsoc_mp_error_string(int errno);

#endif
