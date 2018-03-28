#include "memcpy_userspace.h"
#include "gzll-syscall.h"

#include <string.h>

const char *gzll_kernel_info = "0.1";

void gzll_syscall_get_kernelinfo(struct gzll_syscall *syscall) {
    char *string_userspace = (char*) syscall->param[0];
    gzll_memcpy_to_userspace((void*) string_userspace,
            (void*) gzll_kernel_info, strlen(gzll_kernel_info)+1);
}
