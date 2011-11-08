/*
 * barrier.h
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#ifndef BARRIER_H_
#define BARRIER_H_

#include "mutex.h"
#include "condition.h"

typedef struct {
	mutex_t lock;
	unsigned int volatile count;
	unsigned int volatile cores;
	condition_t cond;
} barrier_t;

extern void barrier_init(barrier_t *barrier);
extern void barrier(barrier_t *barrier);

#endif /* BARRIER_H_ */
