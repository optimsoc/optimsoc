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

#include "task.h"

#include "trace.h"
#include "scheduler.h"
#include "runtime.h"
#include "vmm.h"

#include "syscalls.h"

#include <errno.h>
#include <stdio.h>
#include <assert.h>

int task_create(task_t *task, char *task_name, thread_attr_t *attr, char* param) {
    thread_t thread;
    int i;

    /* Allocate the task data structure */
    *task = calloc(sizeof(struct task), 1);
    assert(*task != NULL);

    /* Find the task object */
    struct task_object *task_obj = NULL;

    /* search task in task table */
    i = 0;
    while(_apps[i].name != NULL){
        if(!strcmp(_apps[i].name, task_name)){
            task_obj = &_apps[i];
            break;
        }
        i++;
    }

    /* Crash if task cannot be found. TODO: Error handling? */
    assert(task_obj != NULL);

    printf("Create new task '%s'\n", task_name);

    /* We copy the name to the data structure */
    (*task)->name = strndup(task_name, 64);

    (*task)->next_thread_id = 1;

    if (!attr) {
        attr = malloc(sizeof(thread_attr_t));
        thread_attr_init(attr);
    }

    attr->args = param;
    attr->flags &= ~THREAD_FLAG_KERNEL;
    /* thread must be supended until page initialization */
    /* FIXME task can not be created suspended */
    attr->flags |= THREAD_FLAG_CREATE_SUSPENDED;

    if (!attr->identifier) {
        attr->identifier = malloc(64);
        snprintf(attr->identifier, 64, "%s.main", task_name);
    }

    thread_create(&thread, (void*) 0x0, attr);
    (*task)->main_thread = thread;

    /* The first thread gets the first stack, the later ones
     * will get different stacks.
     */
    thread->ctx->regs[0] = 0xdffffffc;

    (*task)->all_threads = malloc(sizeof(list_t));
    list_init((*task)->all_threads);
    list_add_tail((*task)->all_threads, thread);

    (*task)->page_table = list_init(NULL);

    void *start_page = (void*) ((unsigned)task_obj->start & ~PAGEMASK);
    void *end_page   = (void*) ((unsigned)task_obj->end   & ~PAGEMASK);

    printf("Load task, origin %p-%p\n", start_page, end_page);

    for(void* page = start_page; page <= end_page; page += PAGESIZE) {
        struct page_table_entry_t* entry = malloc(sizeof(struct page_table_entry_t));


        void *alloc_page = vmm_alloc_page();

        memcpy(alloc_page, page, PAGESIZE);

        entry->vaddr_base = (void*) (page - start_page);
        entry->paddr_base = alloc_page;

        printf("Copied page %p to %p and mapped to %p\n", page, alloc_page, entry->vaddr_base);

        list_add_tail((*task)->page_table, (void*)entry);
    }

    thread->task = (*task);

    thread->task_local_id = 0;
    thread_resume(thread);

    return thread->id;
}

thread_t task_get_main_thread(task_t task) {
    return task->main_thread;
}

void syscall_task_create(void *args) {
    struct syscall_args_task_create *taskargs;
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    taskargs = vmm_virt2phys(core_ctx->active_thread, args, 4);

    task_t *task = vmm_virt2phys(core_ctx->active_thread, taskargs->task, 4);
    char *task_name = vmm_virt2phys(core_ctx->active_thread, taskargs->task_name, 4);

    task_create(task, task_name, 0, 0);
}
