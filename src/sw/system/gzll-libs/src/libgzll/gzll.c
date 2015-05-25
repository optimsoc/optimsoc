
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

    return syscall(GZLL_SYSCALL_NODE_LOOKUP, (uint32_t) name, len,
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
