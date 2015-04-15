

#include <stdlib.h>

#include <gzll-syscall.h>

#include "syscall.h"

void gzll_get_kernel_version(char **version_string) {
	version_string = malloc(GZLL_KERNEL_VERSION_STRING_MAXLENGTH+1);

	syscall(GZLL_SYSCALL_KERNEL_INFO, (uint32_t) version_string, 0, 0, 0, 0, 0);
}

