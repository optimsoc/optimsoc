#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <optimsoc-mp.h>

#include "gzll.h"

#include "../include/gzll-apps.h"

#include <stdio.h>

uint32_t gzll_rank;
optimsoc_thread_t _gzll_comm_thread;

extern void* _image_gzll_kernel_end;
extern void* _image_gzll_apps_start;
extern void* _image_gzll_apps_end;

struct _gzll_image_layout  _gzll_image_layout
__attribute__((section(".gzll_image_layout"))) = {
        { 'g', 'z', 'l', 'l' }, &_image_gzll_kernel_end,
        &_image_gzll_apps_start, &_image_gzll_apps_end };

extern void *__bss_start;
extern void *end;

extern uint32_t _or1k_heap_start;
extern uint32_t _or1k_heap_end;

void _gzll_print_memory_information() {
    printf("Kernel end: %p\n", &_image_gzll_kernel_end);
    printf("BSS start: %p\n", &__bss_start);
    printf("end symbol: %p\n", &end);
    printf("Image start: %p\n", &_image_gzll_apps_start);
    printf("Image end: %p\n", &_image_gzll_apps_end);
    printf("Heap start: %p\n", _or1k_heap_start);
    printf("Current heap ptr: %p\n", _or1k_heap_end);
}

void main() {
    if (gzll_swapping()) {
        // TODO: Load pages from memory
    }

    gzll_rank = optimsoc_get_ctrank();

    printf("Boot gzll in rank %d (%d total)\n", gzll_rank, optimsoc_get_numct());

//    _gzll_print_memory_information();
    // Initialize variables
    gzll_node_nxtid = 1; // Skip 0 which is the kernel

    // Initialize paging
    gzll_paging_init();

    optimsoc_mp_initialize(0);

    optimsoc_syscall_handler_set(gzll_syscall_handler);

    optimsoc_runtime_boot();
}

void init() {
    communication_init();

    gzll_agent_init();

    struct optimsoc_thread_attr *attr;
    attr = malloc(sizeof(struct optimsoc_thread_attr));
    optimsoc_thread_attr_init(attr);
    attr->identifier = "comm";
    attr->flags |= OPTIMSOC_THREAD_FLAG_KERNEL;
    optimsoc_thread_create(&_gzll_comm_thread, &communication_thread, attr);

    gzll_apps_bootstrap();

}
