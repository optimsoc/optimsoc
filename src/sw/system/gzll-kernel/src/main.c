#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <optimsoc-mp.h>

#include "gzll.h"

#include "../include/gzll-apps.h"

#include <stdio.h>

uint32_t _gzll_rank;
optimsoc_thread_t _gzll_comm_thread;

extern void* _image_gzll_kernel_end;
extern void* _image_gzll_apps_start;
extern void* _image_gzll_apps_end;

struct _gzll_image_layout {
    void *kernel_end;
    void *apps_start;
    void *apps_end;
} _gzll_image_layout __attribute__((section(".gzll_image_layout"))) = {
        &_image_gzll_kernel_end, &_image_gzll_apps_start,
        &_image_gzll_apps_end };

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

    for (uint32_t a = 0; a < gzll_application_table.len; a++) {
        struct gzll_application *app = &gzll_application_table.applications[a];
        printf("App: %s\n", app->identifier);
        struct gzll_task_list *tasklist = app->tasks;
        for (uint32_t t = 0; t < tasklist->len; t++) {
            struct gzll_task *task = &tasklist->tasks[t];
            printf(" - task: %s (%p-%p)\n",task->identifier, task->obj_start, task->obj_end);
            task++;
        }
    }
}
