#include <or1k-support.h>
#include <optimsoc-baremetal.h>

extern void syscall_entry(void);

void main() {
	or1k_exception_handler_add(0xc, syscall_entry);

}
