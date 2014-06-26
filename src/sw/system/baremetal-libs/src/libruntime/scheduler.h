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

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "list.h"
#include "thread.h"

extern struct list_t* all_threads;
extern struct list_t* ready_q;
extern struct list_t* wait_q;

struct optimsoc_scheduler_core {
    thread_t idle_thread;
    thread_t active_thread;
};

extern struct optimsoc_scheduler_core* optimsoc_scheduler_core;

extern void scheduler_init();
extern void scheduler_start();
extern void scheduler_add(thread_t t, struct list_t* q);
extern void schedule();
extern void scheduler_yieldcurrent();
extern void scheduler_suspendcurrent();
extern void scheduler_thread_exit();
extern int scheduler_thread_exists(thread_t thread);

#endif
