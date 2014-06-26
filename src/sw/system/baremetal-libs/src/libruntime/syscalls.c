#include <syscalls.h>

void _optimsoc_runtime_syscalls_init(void) {
    or1k_exception_handler_add(0xc, _optimsoc_runtime_syscall_exception_handler);
}

void _optimsoc_runtime_syscall_exception_handler(void) {

}
