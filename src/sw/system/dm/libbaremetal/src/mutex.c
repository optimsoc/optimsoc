/*
 * mutex.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "mutex.h"
#include "cas.h"

#include <or1k-support.h>

void mutex_init(mutex_t *lock) {
  lock->lock = 0;
}

void mutex_lock(mutex_t *lock) {
	while ( cas((unsigned int)&(lock->lock),0,1) == 1 ) {
		unsigned int i;
		for ( i = 0; i < 20; i++ ) __asm__ __volatile__("l.nop");
	}
}

void mutex_unlock(mutex_t *lock) {
	lock->lock = 0;
}
