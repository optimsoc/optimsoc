/*
 * barrier.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "barrier.h"
#include "arch.h"
#include ARCH_INCL(board.h)
#include ARCH_INCL(utils.h)

void barrier_init(barrier_t *barrier) {
	barrier->count = 0;
	barrier->cores = NUMCORES;
	mutex_init(&(barrier->lock));
	condition_init(&(barrier->cond));
}

void barrier(barrier_t *barrier) {
	mutex_lock(&(barrier->lock));
	barrier->count = barrier->count + 1;

	if ( barrier->count == barrier->cores ) {
		barrier->count = 0;
		condition_broadcast(&barrier->cond);
	} else {
		condition_wait(&barrier->cond,&barrier->lock);
	}

	mutex_unlock(&(barrier->lock));
}
