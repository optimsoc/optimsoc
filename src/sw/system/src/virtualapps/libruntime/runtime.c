#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/optimsoc-runtime.h"

#define SYSCALL_CLASS_RUNTIME 0

#define SYSCALL_TASK_START    1
#define SYSCALL_THREAD_CREATE 2

struct syscall_task_start {
    int id;
    char *name;
    char *alias;
};

struct syscall_thread_create {
    int rv;
    optimsoc_runtime_thread_t *thread;
    const optimsoc_runtime_thread_attr_t *attr;
    void *(*start_routine) (void*);
    void *arg;
};

int optimsoc_runtime_task_start(char *name, char *alias) {
    struct syscall_task_start param;
    param.name = name;
    param.alias = alias;
    syscall(SYSCALL_CLASS_RUNTIME, SYSCALL_TASK_START, &param);
    return param.id;
}

struct _thread_wrapper_call {
    void *(*start_routine) (void*);
    void *arg;
};

void _thread_wrapper(void *arg) {
    // The thread enters here
    struct _thread_wrapper_call *c = arg;

    c->start_routine(c->arg);
    free(c);
}

int optimsoc_runtime_thread_create(optimsoc_runtime_thread_t *thread,
                                   const optimsoc_runtime_thread_attr_t *attr,
                                   void *(*start_routine) (void*),
                                   void *arg) {
    struct syscall_thread_create param;
    param.thread = thread;
    param.attr = attr;
    param.start_routine = (void*) _thread_wrapper;

    struct _thread_wrapper_call *c;
    c = malloc(sizeof(struct _thread_wrapper_call));

    c->arg = arg;
    c->start_routine = start_routine;

    param.arg = c;

    syscall(SYSCALL_CLASS_RUNTIME, SYSCALL_THREAD_CREATE, &param);

    int id = param.rv;
    optimsoc_impure_create(id);

    return 0;
}
