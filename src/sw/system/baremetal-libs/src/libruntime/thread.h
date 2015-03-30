/* Copyright (c) 2012-2013 by the author(s)
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
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#ifndef THREAD_H
#define THREAD_H

#include "include/optimsoc-runtime.h"

#include "context.h"
#include <stdlib.h>
#include <string.h>
#include <list.h>

#include <stdint.h>

/*
 * Thread states.
 */
enum optimsoc_thread_state {
    THREAD_RUNNABLE   = 0,
    THREAD_SUSPENDED  = 1,
    THREAD_TERMINATED = 2
};

/* FLAGS for thread creation */
#define THREAD_FLAG_NO_FLAGS    0x0
#define THREAD_FLAG_IDLE_THREAD 0x80000000
#define THREAD_FLAG_PIN         0x00000100
#define THREAD_FLAG_CORE_MASK   0x000000FF
#define THREAD_FLAG_FORCEID     0x40000000
#define THREAD_FLAG_KERNEL      0x20000000
#define THREAD_FLAG_CREATE_SUSPENDED    0x10000000

struct optimsoc_thread {
    uint32_t id;
    struct arch_thread_ctx_t *ctx;
    void *stack;

    optimsoc_page_dir_t page_dir;

    enum optimsoc_thread_state state;
    uint32_t flags;

    struct optimsoc_thread_attr *attributes;

    /* Thread exit code */
    uint32_t exit_code;

    struct optimsoc_list_t *joinlist;

    char *name;

    void *extra_data;
};

optimsoc_page_dir_t _optimsoc_thread_get_pagedir_current();
int _optimsoc_context_create(optimsoc_thread_t thread,
		void (*start_routine)(void*), void *arg);

void _optimsoc_kthread_handle(void (*f)(void*),void *arg);

#endif
