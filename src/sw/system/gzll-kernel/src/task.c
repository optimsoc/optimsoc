#include "gzll.h"
#include "gzll-apps.h"

#include <optimsoc-runtime.h>

#include <assert.h>

// TODO: load from global memory

void gzll_task_start(char* identifier, struct gzll_task *task) {
    printf("Boot task %s\n", identifier);

    unsigned int size = task->obj_end - task->obj_start;

    printf(" load from %p-%p (%d bytes)\n", task->obj_start, task->obj_end,
           size);

    optimsoc_page_dir_t pdir = optimsoc_vmm_create_page_dir();

    unsigned int num_pages = (size + 8191) >> 13;
    for (unsigned int p = 0; p < num_pages; p++) {
        int alloced = gzll_page_alloc();
        assert(alloced);

        void *from = task->obj_start + p * 8192;
        void *to = (void*) (alloced * 8192);

        unsigned int len = 8192;
        if ((unsigned int) from + len > (unsigned int) task->obj_end) {
            len = (unsigned int) task->obj_end - (unsigned int) from;
        }

        printf(" - load %d bytes from %p to %p\n", len, from, to);
        memcpy(to, from, len);
        optimsoc_vmm_map(pdir, (uint32_t) p + 0x2000, (uint32_t) to);
    }

    optimsoc_thread_t thread = malloc(sizeof(optimsoc_thread_t));
    optimsoc_thread_create(&thread, (void*) 0x2000, 0);
    optimsoc_thread_set_pagedir(thread, pdir);

}

