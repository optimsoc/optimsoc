/*
 * condition.h
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#ifndef CONDITION_H_
#define CONDITION_H_

#include "mutex.h"

typedef struct {
	unsigned int volatile waiting;
	unsigned int volatile signal;
} condition_t;

extern void condition_init(condition_t *cond);
extern void condition_wait(condition_t *cond,mutex_t *lock);
extern void condition_broadcast(condition_t *cond);

#endif /* CONDITION_H_ */
