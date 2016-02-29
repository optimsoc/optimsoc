/* Copyright (c) 2013-2015 by the author(s)
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
 * =================================================================
 *
 * Thread context definition
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <stdint.h>

typedef struct _optimsoc_thread_ctx_t {
    uint32_t regs[32];
    uint32_t pc;
    uint32_t sr;
} _optimsoc_thread_ctx_t;

void _optimsoc_context_init(void);
void _optimsoc_context_replace(_optimsoc_thread_ctx_t *ctx);

void _optimsoc_context_copy(_optimsoc_thread_ctx_t *to,
                            _optimsoc_thread_ctx_t *from);

void _optimsoc_context_save(_optimsoc_thread_ctx_t *ctx);
void _optimsoc_context_restore(_optimsoc_thread_ctx_t *ctx);

/**
 * Transfer to exception
 *
 * This is used to transfer from a kernel thread to exception stack. It is
 * useful to call scheduler function from a kernel thread. The state of the
 * thread at the point of entering is saved. From the program you need to check
 * the return value. It returns 1 right after it is called. When the context is
 * replaced later then, the function returns with 0. Thereby you can then call
 * the scheduler functions in the kernel or continue operation.
 */
int _optimsoc_context_enter_exception(_optimsoc_thread_ctx_t *ctx);

#endif
