#include "arch.h"
#include ARCH_INCL(syscalls.h)

#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count) {
	return arch_syscall_write(fd,buf,count);
}
