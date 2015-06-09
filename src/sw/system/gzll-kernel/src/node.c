#include "gzll.h"
#include "gzll-apps.h"
#include "messages.h"
#include "task.h"

#include "gzll-syscall.h"

#include "memcpy_userspace.h"

#include <optimsoc-runtime.h>
#include <or1k-support.h>

#include <stdio.h>
#include <assert.h>

/** Stores the next node id */
gzll_node_id gzll_node_nxtid;

gzll_node_id gzll_get_nodeid() {
    // This is the one we get assigned
    gzll_node_id nodeid;

    do {
        // Read the current next one
        nodeid = gzll_node_nxtid;
        // Try to increment. If someone else already incremented, this will fail
        // and we retry
    } while (or1k_sync_cas(&gzll_node_nxtid, nodeid, nodeid + 1) != nodeid);

    return nodeid;
}

struct optimsoc_list_t *gzll_node_list;

void gzll_node_add(struct gzll_node *task) {
    if (!gzll_node_list) {
        gzll_node_list = optimsoc_list_init(task);
    } else {
        optimsoc_list_add_tail(gzll_node_list, task);
    }
}

// TODO: load from global memory
void gzll_node_start(uint32_t app_id, char* app_name, uint32_t app_nodeid,
                     char* nodename, struct gzll_task_descriptor *taskdesc) {
    int rv;

    printf("Boot node %s.%s\n", app_name, nodename);

    unsigned int size = taskdesc->obj_end - taskdesc->obj_start;

    printf(" load from %p-%p (%d bytes)\n", taskdesc->obj_start,
           taskdesc->obj_end, size);

    optimsoc_page_dir_t pdir = optimsoc_vmm_create_page_dir();

    unsigned int num_pages = (size + 8191) >> 13;
    for (unsigned int p = 0; p < num_pages; p++) {
        unsigned int alloced = gzll_page_alloc();
        assert(alloced);

        void *from = taskdesc->obj_start + p * 8192;
        void *to = (void*) (alloced * 8192);

        unsigned int len = 8192;
        if ((unsigned int) from + len > (unsigned int) taskdesc->obj_end) {
            len = (unsigned int) taskdesc->obj_end - (unsigned int) from;
        }

        printf(" - load %d bytes from %p to %p\n", len, from, to);
        memcpy(to, from, len);

        rv = optimsoc_vmm_map(pdir, (uint32_t) (p << 13) + 0x2000,
                              (uint32_t) to);
        assert(rv == 1);
    }

    // Create a stack
    unsigned int alloced = gzll_page_alloc();
    assert(alloced);
    optimsoc_vmm_map(pdir, 0xfffffffc, alloced << 13);

    // Generate nodeid
    gzll_node_id nodeid = gzll_get_nodeid();

    struct gzll_node *task = calloc(1, sizeof(struct gzll_node));
    task->id = nodeid;

    /* copy identifier */
    /* get sure string fits in array with delimiter */
    assert(strlen(nodename) < GZLL_TASK_IDENTIFIER_LENGTH);
    strncpy(task->identifier, nodename, GZLL_TASK_IDENTIFIER_LENGTH);

    task->app = gzll_app_get(app_id);
    task->taskid = app_nodeid;
    assert(task->app);

    gzll_node_add(task);

    optimsoc_thread_t thread = malloc(sizeof(optimsoc_thread_t));
    optimsoc_thread_create(&thread, (void*) 0x2000, 0);
    optimsoc_thread_set_pagedir(thread, pdir);
    optimsoc_thread_set_extra_data(thread, (void*) task);

    task->thread = thread;
    task->pagedir = pdir;

    struct gzll_app_taskdir *taskdir = task->app->task_dir;
    taskdir_task_register(taskdir, app_nodeid, nodename, gzll_rank, nodeid);

    // Tell the other ranks
    message_send_node_new(app_id, app_nodeid, nodeid, nodename);
}

void gzll_syscall_self(struct gzll_syscall *syscall) {
    optimsoc_thread_t thread;
    struct gzll_node* task;
    thread = optimsoc_thread_current();
    task = (struct gzll_node*) optimsoc_thread_get_extra_data(thread);

    syscall->output = task->taskid;
}

void gzll_syscall_get_taskid(struct gzll_syscall *syscall) {
    char identifier[64];
    gzll_memcpy_from_userspace((void*) identifier,
                               (void*) syscall->param[0],
                               syscall->param[1]);
    optimsoc_thread_t thread;
    struct gzll_node* node;
    thread = optimsoc_thread_current();
    node = (struct gzll_node*) optimsoc_thread_get_extra_data(thread);

    struct gzll_app *app;
    app = node->app;

    struct gzll_app_taskdir *taskdir = app->task_dir;
    assert(taskdir);

    uint32_t taskid;
    int rv = taskdir_taskid_lookup(taskdir, identifier, &taskid);

    if (rv == 0) {
        syscall->output = 0;
    } else {
        syscall->output = -1;
    }

    gzll_memcpy_to_userspace((void*) syscall->param[2], &taskid, 4);
}

// Todo: Move to better place
void gzll_syscall_alloc_page(struct gzll_syscall *syscall) {

    optimsoc_thread_t thread;
    struct gzll_node* task;
    thread = optimsoc_thread_current();

    optimsoc_page_dir_t pdir = optimsoc_thread_get_pagedir(thread);

    uint32_t vaddr = syscall->param[0];
    int size = syscall->param[1];

    while (size > 0) {
        uint32_t alloced = gzll_page_alloc();
        assert(alloced);

        optimsoc_vmm_map(pdir, vaddr, alloced << 13);

        printf("alloc new page %p at %p\n", vaddr, alloced * 8192);

        vaddr += 0x2000;
        size -= 0x2000;
    }

    syscall->output = vaddr;
}

void gzll_task_suspend(struct gzll_task *task) {
    assert(task != NULL);

    if (task->state == GZLL_TASK_ACTIVE) {
        optimsoc_thread_remove(task->thread);
        task->state = GZLL_TASK_SUSPENDED;
    }
}

void gzll_task_resume(struct gzll_task *task) {
    assert(task != NULL);

    if (task->state == GZLL_TASK_SUSPENDED) {
        optimsoc_thread_add(task->thread);
        task->state = GZLL_TASK_ACTIVE;
    }
}

struct gzll_task * gzll_task_task_fetch
(uint32_t remote_tile, void *remote_addr) {

    dma_transfer_handle_t dma_handle;
    struct gzll_task remote_task;

    struct gzll_task *local_task = malloc(sizeof(struct gzll_task));
    assert(local_task != NULL);

    optimsoc_dma_transfer(&remote_task, remote_tile, remote_addr,
                          sizeof(struct gzll_task), REMOTE2LOCAL);

    /* id */
    local_task->id = remote_task.id;
    /* identifier */
    strcpy(local_task->identifier, remote_task.identifier);

    /* app */
    local_task->app = malloc(sizeof(struct gzll_app));
    assert(local_task->app != NULL);
    optimsoc_dma_transfer(local_task->app, remote_tile, remote_task.app,
                          sizeof(struct gzll_app), REMOTE2LOCAL);

    /* state */
    assert(remote_task.state == GZLL_TASK_SUSPENDED);
    local_task->state = GZLL_TASK_SUSPENDED;

    /* pagedir */
    local_task->pagedir = optimsoc_page_dir_copy(remote_tile,
                                                 remote_task.pagedir);

    /* thread */
    local_task->thread = optimsoc_thread_dma_copy(remote_tile,
                                                  remote_task.thread);
    optimsoc_thread_set_pagedir(local_task->thread, local_task->pagedir);
    optimsoc_thread_set_extra_data(local_task->thread,
                                   (void*) local_task);

    /* TODO taskdir */
    /* TODO notify other ranks */

    return local_task;
}
