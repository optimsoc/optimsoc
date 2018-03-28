
#include <optimsoc-runtime.h>

#include <assert.h>

void gzll_memcpy_from_userspace(void* dest, void* src, size_t size) {
    void *psrc;
    optimsoc_page_dir_t dir = optimsoc_thread_get_pagedir(optimsoc_thread_current());

    int rv = optimsoc_vmm_virt2phys(dir, (uint32_t) src, (uint32_t* )&psrc);

    assert(rv); // TODO: allocate page if this fails

    memcpy(dest, psrc, size);
}

void gzll_memcpy_to_userspace(void* dest, void* src, size_t size) {
    void *pdest;
    optimsoc_page_dir_t dir = optimsoc_thread_get_pagedir(optimsoc_thread_current());

    int rv = optimsoc_vmm_virt2phys(dir, (uint32_t) dest, (uint32_t* )&pdest);

    assert(rv); // TODO: allocate page if this fails

    memcpy(pdest, src, size);

}
