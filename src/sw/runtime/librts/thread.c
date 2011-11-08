/*
 * thread.c
 *
 *  Created on: Aug 10, 2011
 *      Author: wallento
 */

#include "thread.h"

void thread_yield() {
	arch_syscall_thread_yield();
}
