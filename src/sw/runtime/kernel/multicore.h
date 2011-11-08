/*
 * multicore.h
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#ifndef MULTICORE_H_
#define MULTICORE_H_

#include "barrier.h"

extern barrier_t multicore_boot_barrier;

void boot_barrier();

void multicore_init_early();
void multicore_init_late();

#endif /* MULTICORE_H_ */
