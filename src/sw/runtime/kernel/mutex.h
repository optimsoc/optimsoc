/*
 * mutex.h
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#ifndef MUTEX_H_
#define MUTEX_H_

typedef struct {
	unsigned int lock;
} mutex_t;

extern void mutex_init(mutex_t *lock);
extern void mutex_lock(mutex_t *lock);
extern void mutex_unlock(mutex_t *lock);

#endif /* MUTEX_H_ */
