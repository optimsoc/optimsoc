#include "scheduler.h"
#include <context.h>

unsigned int *exception_stack;
arch_thread_ctx_t *exception_ctx;

extern unsigned int _stack;

int main() {
	exception_stack = &_stack;
	exception_ctx = (arch_thread_ctx_t*) 0x4;
	scheduler_init();

	scheduler_start();
}
