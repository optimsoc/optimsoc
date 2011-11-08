/*
 * multicore.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "multicore.h"
#include "arch.h"
#include ARCH_INCL(board.h)
#include ARCH_INCL(utils.h)
#include "scheduler.h"

barrier_t multicore_boot_barrier;

void boot_barrier() {
	barrier(&multicore_boot_barrier);
}

void multicore_init_early() {
	// Initialize stuff
	// uart_init();
	mutex_init(&scheduler_mutex);

	arch_multicore_init();
}

void multicore_init_late() {
	// Set barrier parameters, we own the lock!
	multicore_boot_barrier.count = 0; // We will set our value later
	multicore_boot_barrier.cores = NUMCORES;
	condition_init(&(multicore_boot_barrier.cond));

	// Free barrier lock, so that the other cores can enter
	mutex_unlock(&multicore_boot_barrier.lock);
}
