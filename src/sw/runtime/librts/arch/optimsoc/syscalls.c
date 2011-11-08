#define SYSCALL_YIELD	     0x1
#define SYSCALL_SUSPEND	     0x2
#define SYSCALL_ALLOC_PAGES  0x3
#define SYSCALL_WRITE        0x4

#define SYSCALL_PROCTERM	 0x5

#define SYSCALL_DMA_ALLOC    0x6
#define SYSCALL_THREAD_YIELD 0x7

#define SYSCALL_SVC_REQUEST  0x8

#include <unistd.h>

void arch_syscall_yield() {
	  asm("l.addi\tr11,r0,%0": :"K" (SYSCALL_YIELD));
	  asm("l.sys\t0x1");
}

void arch_syscall_suspend() {
	  asm("l.addi\tr11,r0,%0": :"K" (SYSCALL_SUSPEND));
	  asm("l.sys\t0x1");
}

void arch_syscall_proc_terminate() {
	  asm("l.addi\tr11,r0,%0": :"K" (SYSCALL_PROCTERM));
	  asm("l.sys\t0x1");
}

void arch_syscall_alloc_pages(void* vaddr,unsigned int n) {
	  asm("l.addi\tr11,r0,%0": :"K" (SYSCALL_ALLOC_PAGES));
	  asm("l.sys\t0x1");
}

void arch_syscall_dma_alloc() {
	asm("l.addi\tr11,r0,%0" : : "K" (SYSCALL_SVC_REQUEST));
	asm("l.sys\t0x1");
}

ssize_t arch_syscall_write(int fd, const void *buf, size_t count) {
	ssize_t sz;
	__asm__("l.addi r11,r0,%0": :"K" (SYSCALL_WRITE));
	__asm__("l.sys  0x1");
	__asm__("l.add  %0,r11,r0" : "=r" (sz) : );
	return sz;
}

void arch_syscall_thread_yield() {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_THREAD_YIELD));
	asm volatile("l.sys 1");
}
