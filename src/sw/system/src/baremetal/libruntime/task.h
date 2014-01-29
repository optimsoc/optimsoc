/* Copyright (c) 2013 by the author(s)
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

#ifndef APP_H_
#define APP_H_

#include "thread.h"

/**
 * Task descriptor
 *
 * A task is the central unit of computation.
 * On the organizational level, several tasks form a node,
 * but they do not share resources whatsoever.
 *
 * A task is similar to a process. It has resources allocated
 * and may contain several threads. The task itself is not the
 * unit of execution itself, but at least a main thread is started
 * that executes the tasks function.
 */
struct task {
    /** A task identifier, mainly for debugging */
    char *name;

    /** The main thread of execution */
    thread_t main_thread;
    /** List of all threads */
    list_t *all_threads;

    /** Next thread id
     *
     * Each thread has an id local to the task. This can be extracted
     * in the user level via the stack. To properly set up the stacks
     * we need to know which the next free thread id is.
     */
    unsigned int next_thread_id;

    /** The tasks page table */
    struct list_t *page_table;
};

struct task_object {
    char *name;
    void *start;
    void *end;
};

extern struct task_object _apps[];

typedef struct task* task_t;

int task_create(task_t *app, char *app_name, thread_attr_t *attr, char* param);
thread_t task_get_main_thread(task_t app);

#endif /* APP_H_ */
