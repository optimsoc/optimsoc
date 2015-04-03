#include <assert.h>

#include <optimsoc-baremetal.h>
#include "include/optimsoc-runtime.h"
#include "syscalls.h"

/*! Pointer to the array of structs which contains one syscall per core. */
struct optimsoc_syscall *_optimsoc_syscall_data;

optimsoc_syscall_handler_fptr _optimsoc_syscall_handler;

void _optimsoc_syscalls_init() {
	// Set the exception handler for syscall exceptions
    or1k_exception_handler_add(0xc, &_optimsoc_sycall_entry);

	void* c = calloc(or1k_numcores(), sizeof(struct optimsoc_syscall));
	assert(c);
	_optimsoc_syscall_data = (struct optimsoc_syscall*) c;
}

void optimsoc_syscall_handler_set(optimsoc_syscall_handler_fptr handler) {
    _optimsoc_syscall_handler = handler;
}
