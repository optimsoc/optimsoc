/*
 * mmap.c
 *
 *  Created on: Nov 25, 2010
 *      Author: wallento
 */

#include <sys/types.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	__asm__("l.sys 1");
	return 0;
}

int munmap(void *addr, size_t length) {
	__asm__("l.sys 1");
	return 0;
}
