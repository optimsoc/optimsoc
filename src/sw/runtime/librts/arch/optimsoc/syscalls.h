/*
 * syscall.h
 *
 *  Created on: Nov 20, 2010
 *      Author: wallento
 */

#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <unistd.h>

void arch_syscall_yield();
void arch_syscall_suspend();
void arch_syscall_alloc_pages(void* vaddr,unsigned int n);
ssize_t arch_syscall_write(int fd, const void *buf, size_t count);

void* arch_syscall_dma_alloc();

void arch_syscall_thread_yield();

#endif /* SYSCALL_H_ */
