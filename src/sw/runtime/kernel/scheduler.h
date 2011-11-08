/*
 * scheduler.h
 *
 *  Created on: Nov 20, 2010
 *      Author: wallento
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "thread.h"
#include "process.h"
#include "mutex.h"

#define DEBUG_SCHEDULER	0

extern thread_t *active_thread[NUMCORES];
extern process_t *active_process;

extern mutex_t scheduler_mutex;

void scheduler_init();
void scheduler_add(thread_t *thread);
void scheduler_remove(thread_t *thread);

/* syscall entries */
void scheduler_yield(arch_thread_ctx_t* ctx);
void scheduler_suspend(arch_thread_ctx_t* ctx);

// future syscall entries
void scheduler_resume(unsigned int threadid);
thread_id_t scheduler_threadid();
void scheduler_exit();

void schedule();
void scheduler_start();

void svc_scheduler_handler(unsigned int req,arch_thread_ctx_t *ctx);

#endif /* SCHEDULER_H_ */
