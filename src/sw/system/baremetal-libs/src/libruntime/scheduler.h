/* Copyright (c) 2012-2015 by the author(s)
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

#include "include/optimsoc-runtime.h"

#include "list.h"
#include "thread.h"

extern struct optimsoc_list_t* all_threads;
extern struct optimsoc_list_t* ready_q;
extern struct optimsoc_list_t* wait_q;

void _optimsoc_scheduler_init();
void _optimsoc_scheduler_start();
void _optimsoc_scheduler_add(optimsoc_thread_t t, struct optimsoc_list_t* q);
void _optimsoc_schedule();
void _optimsoc_scheduler_yieldcurrent();
void _optimsoc_scheduler_suspendcurrent();

optimsoc_thread_t _optimsoc_scheduler_get_current(void);

#endif
