#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <optimsoc-mp.h>

#include "gzll.h"

#include "../include/gzll-apps.h"

#include <stdio.h>

uint32_t _gzll_rank;
optimsoc_thread_t _gzll_comm_thread;

void main() {
    if (gzll_swapping()) {
        // TODO: Load pages from memory
    }

    // Initialize paging
    gzll_paging_init();

    optimsoc_mp_initialize(0);

    _gzll_rank = optimsoc_get_ctrank();

    printf("Boot gzll in node %d (%d total)\n", _gzll_rank, optimsoc_get_numct());

    optimsoc_runtime_boot();
}

void init() {
    communication_init();

    struct optimsoc_thread_attr attr;
    optimsoc_thread_attr_init(&attr);
    attr.identifier = "comm";
    attr.flags |= OPTIMSOC_THREAD_FLAG_KERNEL;
    optimsoc_thread_create(&_gzll_comm_thread, &communication_thread, &attr);

    struct gzll_application *app = &gzll_application_table[0];
    while (app->identifier) {
        printf("App: %s\n", app->identifier);
        struct gzll_task *task = app->tasks;
        while (task->identifier) {
            printf(" - task: %s (%p-%p)\n",task->identifier, task->obj_start, task->obj_end);
            task++;
        }
        app++;
    }
}
