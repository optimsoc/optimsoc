#include <optimsoc-baremetal.h>
#include "include/optimsoc-runtime.h"
#include "syscalls.h"

struct optimsoc_runtime_syscall *_optimsoc_runtime_syscall_data;

void _optimsoc_runtime_syscalls_init() {
	// By default no exception handler is set
	or1k_exception_handler_add(0xc, (or1k_exception_handler_fptr) -1);

	void* c = calloc(or1k_numcores(), sizeof(struct optimsoc_runtime_syscall));
	_optimsoc_runtime_syscall_data = (struct optimsoc_runtime_syscall*) c;
}

void optimsoc_runtime_syscall_handler_set(
		optimsoc_runtime_syscall_handler_fptr handler) {
	or1k_exception_handler_add(0xc, &_optimsoc_runtime_sycall_entry);
}
