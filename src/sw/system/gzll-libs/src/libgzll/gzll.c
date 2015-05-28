
#include <stdlib.h>
#include <string.h>

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

struct _gzll_endpoint {
    uint32_t node;
    uint32_t port;
};

int gzll_mp_endpoint_create(gzll_mp_endpoint_t *ep, uint32_t port,
                            gzll_endpoint_type buffer_type,
                            uint32_t buffer_size, uint32_t max_elem_size) {
    struct _gzll_endpoint *eph = malloc(sizeof (struct _gzll_endpoint));
    *ep = (gzll_mp_endpoint_t) eph;

    eph->node = gzll_self();
    eph->port = port;

    return syscall(GZLL_SYSCALL_ENDPOINT_CREATE, port, (uint32_t) buffer_type,
                   buffer_size, max_elem_size, 0, 0);
}

int gzll_mp_endpoint_get(gzll_mp_endpoint_t *ep, uint32_t node, uint32_t port) {
    struct _gzll_endpoint *eph = malloc(sizeof (struct _gzll_endpoint));
    *ep = (gzll_mp_endpoint_t) eph;

    eph->node = node;
    eph->port = port;

    return syscall(GZLL_SYSCALL_ENDPOINT_GET, node, port, 0, 0, 0, 0);
}

int gzll_mp_channel_connect(gzll_mp_endpoint_t from, gzll_mp_endpoint_t to) {
    struct _gzll_endpoint *fep, *tep;

    fep = (struct _gzll_endpoint*) from;
    tep = (struct _gzll_endpoint*) to;

    return syscall(GZLL_SYSCALL_CHANNEL_CONNECT, fep->node, fep->port,
                   tep->node, tep->port, 0, 0);
}

int gzll_mp_channel_send(gzll_mp_endpoint_t from, gzll_mp_endpoint_t to,
                         uint8_t* buffer, uint32_t size) {
}

int gzll_mp_channel_recv(gzll_mp_endpoint_t ep, uint8_t* buffer,
                         uint32_t buffer_size, uint32_t *received_size) {

}
