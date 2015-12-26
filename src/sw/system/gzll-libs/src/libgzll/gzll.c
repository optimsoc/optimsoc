
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "include/gzll.h"
#include <gzll-syscall.h>

#include "syscall.h"

void gzll_get_kernel_version(char **version_string) {
    *version_string = malloc(GZLL_KERNEL_VERSION_STRING_MAXLENGTH+1);

    syscall(GZLL_SYSCALL_KERNEL_INFO, (uint32_t) *version_string, 0, 0, 0, 0, 0);
}

int gzll_lookup_nodeid(char *name, gzll_node_id *nodeid) {
    uint32_t len = strlen(name) + 1;

    return syscall(GZLL_SYSCALL_TASK_LOOKUP, (uint32_t) name, len,
                   (uint32_t) nodeid, 0, 0, 0);
}

gzll_node_id _gzll_self;
uint8_t _gzll_self_identified;

gzll_node_id gzll_self(void) {
    if (!_gzll_self_identified) {
        _gzll_self = syscall(GZLL_SYSCALL_SELF, 0, 0, 0, 0, 0, 0);
        _gzll_self_identified = 1;
    }
    return _gzll_self;
}

struct gzll_mp_endpoint {
    uint32_t node;
    uint32_t port;
};

struct gzll_mp_channel {
    struct gzll_mp_endpoint *from;
    struct gzll_mp_endpoint *to;
};

int gzll_mp_endpoint_create(gzll_mp_endpoint_t *ep, uint32_t port,
                            gzll_endpoint_type buffer_type,
                            uint32_t buffer_size, uint32_t max_elem_size) {
    *ep = malloc(sizeof (struct gzll_mp_endpoint));

    (*ep)->node = gzll_self();
    (*ep)->port = port;

    return syscall(GZLL_SYSCALL_ENDPOINT_CREATE, port, (uint32_t) buffer_type,
                   buffer_size, max_elem_size, 0, 0);
}

int gzll_mp_endpoint_get(gzll_mp_endpoint_t *ep, uint32_t node, uint32_t port) {
    *ep = malloc(sizeof (struct gzll_mp_endpoint));

    (*ep)->node = node;
    (*ep)->port = port;

    return syscall(GZLL_SYSCALL_ENDPOINT_GET, node, port, 0, 0, 0, 0);
}

int gzll_mp_channel_connect(gzll_mp_endpoint_t from, gzll_mp_endpoint_t to,
                            gzll_mp_channel_t *channel) {
    assert(from);
    assert(to);
    assert(channel);

    *channel = malloc(sizeof(struct gzll_mp_channel));
    assert(*channel);

    (*channel)->from = from;
    (*channel)->to = to;

    return syscall(GZLL_SYSCALL_CHANNEL_CONNECT, from->node, from->port,
                   to->node, to->port, 0, 0);
}

int gzll_mp_channel_send(gzll_mp_channel_t channel, uint8_t* buffer,
                         uint32_t size) {
}

int gzll_mp_channel_recv(gzll_mp_endpoint_t ep, uint8_t* buffer,
                         uint32_t buffer_size, uint32_t *received_size) {

}
