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

#include "context.h"
#include <stdlib.h>
#include <string.h>
#include <list.h>

/* FLAGS for thread creation */
#define THREAD_FLAG_NO_FLAGS    0x0
#define THREAD_FLAG_IDLE_THREAD 0x80000000
#define THREAD_FLAG_PIN         0x00000100
#define THREAD_FLAG_CORE_MASK   0x000000FF
#define THREAD_FLAG_FORCEID     0x40000000
#define THREAD_FLAG_KERNEL      0x20000000
#define THREAD_FLAG_CREATE_SUSPENDED    0x10000000

/* Migration messages defines */
#define TM_MSG_ID_LSB 0x0
#define TM_MSG_ID_MSB 0x7
#define TM_MSG_REQ           0x1
#define TM_MSG_REQ_ACK       0x2
#define TM_MSG_REQ_NACK      0x3
#define TM_MSG_PAGE          0x4
#define TM_MSG_PAGE_END      0x5
#define TM_MSG_MIGRATE_ACK   0x6
#define TM_MSG_MIGRATE_NACK  0x7


/* Thread identifier */
typedef unsigned int tid_t;

/*
 * Thread states.
 */
enum thread_state {
    THREAD_RUNNABLE   = 0,
    THREAD_SUSPENDED  = 1,
    THREAD_TERMINATED = 2
};

struct thread_attr {
    void *args;
    unsigned int flags;
    unsigned int force_id;
    char *identifier;
};

/*
 * Thread control block.
 */
struct thread_t {
    tid_t id;
    struct arch_thread_ctx_t *ctx;
    void *stack;

    enum thread_state state;
    unsigned int flags;

    struct thread_attr *attributes;

    /* Thread exit code */
    unsigned int exit_code;

    struct list_t *joinlist;

    struct task *task;
    unsigned int task_local_id;

    char *name;
};

typedef struct thread_t *thread_t;
typedef struct thread_attr thread_attr_t;

typedef struct thread_recv {
    thread_t remote_thread;
    list_t *remote_page_table;
    list_t *page_pool;
} thread_recv_t;

void thread_attr_init(thread_attr_t *attr);
int thread_create(thread_t* t,void (*func)(void*), thread_attr_t *attr);
int thread_send(thread_t t, unsigned int dest_tileid);
void thread_receive(unsigned int *buffer,int len);
int thread_migrate(unsigned int remote_tileid, void* thread_foreign_addr, struct list_t *remote_thread_page_table, struct list_t *page_pool);
void thread_destroy(thread_t thread);
int arch_context_create(thread_t thread, void (*start_routine)(void*), void *arg);
void thread_yield();
void thread_suspend();
void thread_resume(thread_t t);
thread_t thread_self();
void thread_join(thread_t thread);
void thread_exit();

void syscall_thread_create(void* args);

#endif
