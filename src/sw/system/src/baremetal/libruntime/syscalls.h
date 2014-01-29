#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "task.h"

struct syscall_args_task_create {
    task_t *task;
    char *task_name;
};

void syscall_task_create(void *arg);


#endif /* SYSCALLS_H_ */
