#include "gzll.h"
#include "gzll-apps.h"
#include "messages.h"
#include "node.h"

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

void gzll_node_add(struct gzll_node *node) {
    if (!gzll_node_list) {
        gzll_node_list = optimsoc_list_init(node);
    } else {
        optimsoc_list_add_tail(gzll_node_list, node);
    }
}

int gzll_node_remove(struct gzll_node *node)
{
    if (gzll_node_list != NULL) {
        if (optimsoc_list_remove(gzll_node_list, node) == 1) {
            /* success */
            return 0;
        }
    }

    /* failed */
    return -1;
}

struct gzll_node *gzll_node_find(uint32_t id)
{
    optimsoc_list_iterator_t iter;

    struct gzll_node *node = (struct gzll_node*) optimsoc_list_first_element(
        gzll_node_list, &iter);

    while (node != NULL) {
        if (node->id == id) {
            return node;
        }

        node = optimsoc_list_next_element(gzll_node_list, &iter);
    }

    return NULL;
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
    assert(strlen(nodename) < GZLL_NODE_IDENTIFIER_LENGTH);
    strncpy(task->identifier, nodename, GZLL_NODE_IDENTIFIER_LENGTH);

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

void gzll_node_suspend(struct gzll_node *node) {
    assert(node != NULL);

    if (node->state == GZLL_NODE_ACTIVE) {
        optimsoc_thread_remove(node->thread);
        node->state = GZLL_NODE_SUSPENDED;
    }
}

void gzll_node_resume(struct gzll_node *node) {
    assert(node != NULL);

    if (node->state == GZLL_NODE_SUSPENDED) {
        optimsoc_thread_add(node->thread);
        node->state = GZLL_NODE_ACTIVE;
    }
}

struct gzll_node * gzll_node_fetch
(uint32_t remote_tile, void *remote_addr) {

    dma_transfer_handle_t dma_handle;
    struct gzll_node remote_node;

    struct gzll_node *local_node = malloc(sizeof(struct gzll_node));
    assert(local_node != NULL);

    optimsoc_dma_transfer(&remote_node, remote_tile, remote_addr,
                          sizeof(struct gzll_node), REMOTE2LOCAL);

    /* id */
    local_node->id = remote_node.id;
    /* identifier */
    strncpy(local_node->identifier, remote_node.identifier,
            GZLL_NODE_IDENTIFIER_LENGTH);

    /* app */
    local_node->app = malloc(sizeof(struct gzll_app));
    assert(local_node->app != NULL);
    optimsoc_dma_transfer(local_node->app, remote_tile, remote_node.app,
                          sizeof(struct gzll_app), REMOTE2LOCAL);

    /* state */
    assert(remote_node.state == GZLL_NODE_SUSPENDED);
    local_node->state = GZLL_NODE_SUSPENDED;

    /* pagedir */
    local_node->pagedir = optimsoc_vmm_dir_copy(remote_tile,
                                                remote_node.pagedir,
                                                &gzll_page_alloc);

    /* thread */
    local_node->thread = optimsoc_thread_dma_copy(remote_tile,
                                                  remote_node.thread);
    optimsoc_thread_set_pagedir(local_node->thread, local_node->pagedir);
    optimsoc_thread_set_extra_data(local_node->thread,
                                   (void*) local_node);


    /* TODO nodedir */
    /* TODO notify other ranks */

    return local_node;
}
