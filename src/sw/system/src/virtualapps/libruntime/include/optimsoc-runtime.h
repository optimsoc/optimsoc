#ifndef OPTIMSOC_RUNTIME_H_
#define OPTIMSOC_RUNTIME_H_

typedef unsigned int optimsoc_runtime_thread_t;

typedef struct {
    char *name;
} optimsoc_runtime_thread_attr_t;

int optimsoc_runtime_task_start(char *name, char *alias);

int optimsoc_runtime_thread_create(optimsoc_runtime_thread_t *thread,
                                   const optimsoc_runtime_thread_attr_t *attr,
                                   void *(*start_routine) (void*),
                                   void *arg);

optimsoc_runtime_thread_t optimsoc_runtime_thread_self();

#endif /* OPTIMSOC_RUNTIME_H_ */
