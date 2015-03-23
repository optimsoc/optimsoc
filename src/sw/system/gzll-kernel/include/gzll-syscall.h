#ifndef __GZLL_SYSCALL_H__
#define __GZLL_SYSCALL_H__

#include <inttypes.h>

/**
 * Syscall structure
 *
 * This is the structure that defines the ABI between the userspace and the kernel.
 *
 * A system call is defined by the identifier and can contain up to four
 * parameters. If a system call has more parameters those need to be transfered
 * via a data structure in the user memory.
 */
struct gzll_syscall {
    uint32_t id; /*!< Identifier of the system call */
    uint32_t param[5]; /*!< Five parameters to the system call */
    uint32_t output; /*!< Output/return value */
};

static const uint32_t GZLL_KERNEL_VERSION_STRING_MAXLENGTH = 32;

/**
 * Kernel info
 */
struct gzll_kernel_info {
    char *version_string;
};

/**
 *  Get kernel information
 *
 *  - Parameter 0: Pointer to gzll_kernel_info structure
 *  - Output: 0
 */
static const uint32_t GZLL_SYSCALL_KERNEL_INFO = 0;

/** Allocate new page for user application */
static const uint32_t GZLL_SYSCALL_VMM_ALLOC_PAGE = 1;

#endif
