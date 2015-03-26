#include <optimsoc-baremetal.h>
#include "syscalls.h"

void _optimsoc_runtime_syscalls_init() {
	or1k_exception_handler_add(0xc, &_optimsoc_runtime_sycall_entry);
}
