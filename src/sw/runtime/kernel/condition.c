/*
 * condition.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "condition.h"
#include "arch.h"
#include ARCH_INCL(utils.h)

void condition_init(condition_t *cond) {
	cond->signal  = 0;
	cond->waiting = 0;
}

void condition_wait(condition_t *cond,mutex_t *lock) {

	cond->waiting = cond->waiting + 1;
	mutex_unlock(lock);

	while ( cond->signal == 0 ) {
		unsigned int i;
		for ( i = 0; i < 20; ++i ) __asm__ __volatile__("l.nop 0xf");
	}

	mutex_lock(lock);
	cond->waiting = cond->waiting - 1;
	if ( cond->waiting == 0 ) {
		cond->signal = 0;
	}
}

void condition_broadcast(condition_t *cond) {
	cond->signal = 1;
}
