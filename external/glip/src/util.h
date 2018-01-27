/* Copyright (c) 2014-2015 by the author(s)
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
 * ============================================================================
 *
 * Utility functions used in GLIP and its backends.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <libglip.h>
#include "cbuf.h"

#include <time.h>

/*
 * The two functions below are "as seen in the Linux kernel".
 * see include/linux/math64.h and include/linux/time.h in the linux kernel tree
 */
static inline uint32_t __iter_div_u64_rem(uint64_t dividend, uint32_t divisor,
                                          uint64_t *remainder)
{
    uint32_t ret = 0;

    while (dividend >= divisor) {
        /* The following asm() prevents the compiler from
           optimising this loop into a modulo operation. */
        asm("" : "+rm"(dividend));
        dividend -= divisor;
        ret++;
    }
    *remainder = dividend;
    return ret;
}

#define NSEC_PER_SEC 1000000000L
static inline void timespec_add_ns(struct timespec *a, uint64_t ns)
{
    a->tv_sec += __iter_div_u64_rem(a->tv_nsec + ns, NSEC_PER_SEC, &ns);
    a->tv_nsec = ns;
}

static inline int max(int a, int b)
{
    return a > b ? a : b;
}

static inline int min(int a, int b)
{
    return a > b ? b : a;
}

int gl_util_connect_to_host(struct glip_ctx *ctx, const char *hostname,
                            unsigned int port, int *socketfd);
int gl_util_fd_nonblock(struct glip_ctx *ctx, int fd);

int gl_util_popen(const char *file, char *const argv[], int *infd, int *outfd);
int gl_util_pclose(pid_t pid);

int gb_util_cbuf_read(struct cbuf *buf, size_t size, uint8_t *data,
                      size_t *size_read);
int gb_util_cbuf_read_b(struct cbuf *buf, size_t size, uint8_t *data,
                        size_t *size_read, unsigned int timeout);
int gb_util_cbuf_write(struct cbuf *buf, size_t size, uint8_t *data,
                       size_t *size_written);
int gb_util_cbuf_write_b(struct cbuf *buf, size_t size, uint8_t *data,
                         size_t *size_written, unsigned int timeout);

#endif /* __UTIL_H__ */
