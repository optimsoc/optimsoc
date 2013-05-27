/*
 * mutex.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include <or1k-support.h>

extern uint32_t cas(void*, uint32_t, uint32_t);

struct mutex {
    uint32_t size;
    uint32_t data;
};

void mutex_init(struct mutex *mutex);
void mutex_lock(struct mutex *mutex);
void mutex_unlock(struct mutex *mutex);

void mutex_init(struct mutex *mutex) {
    mutex->size = sizeof(mutex->data);
    mutex->data = 0;
}

void mutex_lock(struct mutex *lock) {
	while ( cas((void*)&(lock->data),0,1) == 1 ) {
		unsigned int i;
		for ( i = 0; i < 20; i++ ) __asm__ __volatile__("l.nop");
	}
}

void mutex_unlock(struct mutex *lock) {
	lock->data = 0;
}
