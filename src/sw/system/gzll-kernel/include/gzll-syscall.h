/* Copyright (c) 2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

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