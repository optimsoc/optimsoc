#include "gzll.h"
#include "gzll-apps.h"
#include "messages.h"

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

// TODO: load from global memory
void gzll_task_start(uint32_t app_id, char* app_name, uint32_t app_nodeid,
                     struct gzll_task_descriptor *task) {
    int rv;

    printf("Boot task %s.%s\n", app_name, task->identifier);

    unsigned int size = task->obj_end - task->obj_start;

    printf(" load from %p-%p (%d bytes)\n", task->obj_start, task->obj_end,
           size);

    optimsoc_page_dir_t pdir = optimsoc_vmm_create_page_dir();

    unsigned int num_pages = (size + 8191) >> 13;
    for (unsigned int p = 0; p < num_pages; p++) {
        unsigned int alloced = gzll_page_alloc();
        assert(alloced);

        void *from = task->obj_start + p * 8192;
        void *to = (void*) (alloced * 8192);

        unsigned int len = 8192;
        if ((unsigned int) from + len > (unsigned int) task->obj_end) {
            len = (unsigned int) task->obj_end - (unsigned int) from;
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

    optimsoc_thread_t thread = malloc(sizeof(optimsoc_thread_t));
    optimsoc_thread_create(&thread, (void*) 0x2000, 0);
    optimsoc_thread_set_pagedir(thread, pdir);

    // Generate nodeid
    gzll_node_id nodeid = gzll_get_nodeid();

    // TODO: store locally

    // Tell the other ranks
    message_send_node_new(app_id, app_nodeid, nodeid);
}

