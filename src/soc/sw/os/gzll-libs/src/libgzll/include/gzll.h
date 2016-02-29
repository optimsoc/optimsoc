#ifndef __GZLL_H__
#define __GZLL_H__

#include <stdint.h>

void gzll_get_kernel_version(char **version_string);

typedef uint32_t gzll_node_id;
gzll_node_id gzll_self(void);
int gzll_lookup_nodeid(char *name, gzll_node_id *nodeid);

typedef struct gzll_mp_endpoint* gzll_mp_endpoint_t;
typedef struct gzll_mp_channel*  gzll_mp_channel_t;

typedef enum { OPTIMSOC_MP_EP_CONNECTIONLESS = 0,
    OPTIMSOC_MP_EP_CHANNEL = 1 } gzll_endpoint_type;

int gzll_mp_endpoint_create(gzll_mp_endpoint_t *ep, uint32_t port,
                            gzll_endpoint_type buffer_type,
                            uint32_t buffer_size, uint32_t max_element_size);

int gzll_mp_endpoint_get(gzll_mp_endpoint_t *ep, uint32_t node, uint32_t port);

int gzll_mp_channel_connect(gzll_mp_endpoint_t from, gzll_mp_endpoint_t to,
                            gzll_mp_channel_t *channel);

int gzll_mp_channel_send(gzll_mp_channel_t channel, uint8_t* buffer,
                         uint32_t size);

int gzll_mp_channel_recv(gzll_mp_endpoint_t ep, uint8_t* buffer,
                         uint32_t buffer_size, uint32_t *received_size);

#endif
