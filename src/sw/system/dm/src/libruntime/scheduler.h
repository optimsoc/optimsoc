#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "list.h"
#include "thread.h"

extern struct list_t* all_threads;
extern struct list_t* ready_q;
extern struct list_t* wait_q;

extern thread_t active_thread;

extern void scheduler_init();
extern void scheduler_start();
extern void scheduler_add(thread_t t, struct list_t* q);
extern void schedule();
extern void scheduler_yieldcurrent();
extern void scheduler_suspendcurrent();
extern void scheduler_thread_exit();
extern int scheduler_thread_exists(thread_t thread);

#endif
