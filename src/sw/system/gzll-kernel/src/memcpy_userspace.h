#ifndef __MEMCPY_USERSPACE_H__
#define __MEMCPY_USERSPACE_H__

#include <stddef.h>

extern void gzll_memcpy_from_userspace(void* dest, void* src, size_t size);
extern void gzll_memcpy_to_userspace(void* dest, void* src, size_t size);

#endif
