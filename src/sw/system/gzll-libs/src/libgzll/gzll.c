
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
