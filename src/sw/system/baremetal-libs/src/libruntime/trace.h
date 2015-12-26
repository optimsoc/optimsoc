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
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <optimsoc-baremetal.h>
#include "runtime.h"
#include "config.h"

#define TRACE_THREAD_CREATE 0x300
#define TRACE_SCHEDULE      0x301
#define TRACE_RESUME        0x302
#define TRACE_JOINWAIT      0x303
#define TRACE_SUSPEND       0x304
#define TRACE_YIELD         0x305
#define TRACE_ITLBMISS      0x306
#define TRACE_DTLBMISS      0x307
#define TRACE_DTLBPAGEALLOC 0x308
#define TRACE_EXIT          0x309
#define TRACE_LOAD          0x30a

#define TRACE_THREAD_SEND       0x30b
#define TRACE_THREAD_DESTROY    0x30c

static inline void runtime_trace_sendthread(char *name,
                                            unsigned int id,
                                            void *thread,
                                            unsigned int dest) {

    while (*name) {
        OPTIMSOC_TRACE(TRACE_THREAD_SEND,*name);
        name++;
    }
    OPTIMSOC_TRACE(TRACE_THREAD_SEND,0); // Terminate string

    OPTIMSOC_TRACE(TRACE_THREAD_SEND,id);

    OPTIMSOC_TRACE(TRACE_THREAD_SEND,thread);

    OPTIMSOC_TRACE(TRACE_THREAD_SEND,dest);
}

static inline void runtime_trace_destroythread(unsigned int id) {
    OPTIMSOC_TRACE(TRACE_THREAD_DESTROY,id);
}


static inline void runtime_trace_createthread(char *name,
                                              unsigned int id,
                                              void *thread,
                                              void *func) {
    if (runtime_config_get_use_globalids()) {
        optimsoc_trace_defineglobalsection(id,name);
    } else {
        optimsoc_trace_definesection(id,name);
    }

    while (*name) {
        OPTIMSOC_TRACE(TRACE_THREAD_CREATE,*name);
        name++;
    }
    OPTIMSOC_TRACE(TRACE_THREAD_CREATE,0); // Terminate string

    OPTIMSOC_TRACE(TRACE_THREAD_CREATE,id);

    OPTIMSOC_TRACE(TRACE_THREAD_CREATE,thread);

    OPTIMSOC_TRACE(TRACE_THREAD_CREATE,func);
}

static inline void runtime_trace_schedule(unsigned int id) {
    OPTIMSOC_TRACE(TRACE_SCHEDULE,id);
    optimsoc_trace_section((int)id);
}

static inline void runtime_trace_resume(unsigned int id,
                                        unsigned int byid) {
    OPTIMSOC_TRACE(TRACE_RESUME,id);
    OPTIMSOC_TRACE(TRACE_RESUME,byid);
}

static inline void runtime_trace_joinwait(unsigned int id) {
    OPTIMSOC_TRACE(TRACE_JOINWAIT,id);
}

static inline void runtime_trace_suspend(unsigned int id) {
    OPTIMSOC_TRACE(TRACE_SUSPEND,id);
}

static inline void runtime_trace_yield(unsigned int id) {
    OPTIMSOC_TRACE(TRACE_YIELD,id);
}

static inline void runtime_trace_itlb_miss(uint32_t vaddr) {
    OPTIMSOC_TRACE(TRACE_ITLBMISS,vaddr);
}

static inline void runtime_trace_dtlb_miss(uint32_t vaddr) {
    OPTIMSOC_TRACE(TRACE_DTLBMISS,vaddr);
}

static inline void runtime_trace_thread_exit() {
    OPTIMSOC_TRACE(TRACE_EXIT, 1);
}

static inline void runtime_trace_load(unsigned int load) {
    OPTIMSOC_TRACE(TRACE_LOAD, load);
}

#endif /* TRACE_H_ */
