#ifndef THREAD_H_
#define THREAD_H_

#include "arch.h"
#include ARCH_INCL(context.h)

typedef unsigned int thread_id_t;

typedef struct thread_attr_t {

} thread_attr_t;

typedef struct thread_t {
	thread_id_t       id;
	struct process_t  *process;
	arch_thread_ctx_t ctx;
	void              *stack;
	thread_attr_t     *attr;
} thread_t;


int thread_create( thread_t *thread, const thread_attr_t *attr,
		void *(*start_routine)(void*), void *arg);

void thread_yield();
void thread_suspend();
void thread_resume(unsigned int);
thread_id_t thread_self();
void thread_exit(void* value);

void syscall_thread_yield(arch_thread_ctx_t *ctx);

int app_create(void *start,void *end);
void process_terminate(arch_thread_ctx_t *syscall_ctx);

#endif /* THREAD_H_ */
