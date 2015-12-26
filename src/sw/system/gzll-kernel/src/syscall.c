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

#include <stdio.h>

#include <optimsoc-runtime.h>
#include "gzll-syscall.h"

void gzll_syscall_get_kernelinfo(struct gzll_syscall *syscall);
void gzll_syscall_self(struct gzll_syscall *syscall);
void gzll_syscall_get_taskid(struct gzll_syscall *syscall);

void gzll_syscall_endpoint_create(struct gzll_syscall *syscall);
void gzll_syscall_endpoint_get(struct gzll_syscall *syscall);

void gzll_syscall_alloc_page(struct gzll_syscall *syscall);
void gzll_syscall_channel_connect(struct gzll_syscall *syscall);

void gzll_syscall_handler(struct optimsoc_syscall *optimsoc_syscall) {
    struct gzll_syscall *syscall = (struct gzll_syscall*) optimsoc_syscall;

    switch (syscall->id) {
    case GZLL_SYSCALL_THREAD_EXIT:
        optimsoc_thread_exit();
        break;
    case GZLL_SYSCALL_KERNEL_INFO:
        gzll_syscall_get_kernelinfo(syscall);
        break;
    case GZLL_SYSCALL_SELF:
        gzll_syscall_self(syscall);
        break;
    case GZLL_SYSCALL_TASK_LOOKUP:
        gzll_syscall_get_taskid(syscall);
        break;
    case GZLL_SYSCALL_ENDPOINT_CREATE:
        gzll_syscall_endpoint_create(syscall);
        break;
    case GZLL_SYSCALL_ENDPOINT_GET:
        gzll_syscall_endpoint_get(syscall);
        break;
    case GZLL_SYSCALL_VMM_ALLOC_PAGE:
        gzll_syscall_alloc_page(syscall);
        break;
    case GZLL_SYSCALL_CHANNEL_CONNECT:
        gzll_syscall_channel_connect(syscall);
        break;
    default:
        printf("Invalid syscall, kill thread\n");
        optimsoc_thread_exit();
        break;
    }
}
