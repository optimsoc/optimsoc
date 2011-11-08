/*
 * malloc.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "mutex.h"
#include <stddef.h>
#include <stdlib.h>
#include "arch.h"

mutex_t malloc_lock = { 0 };

void *calloc_r(size_t nmemb, size_t size) {
	void *p;
	mutex_lock(&malloc_lock);
	p = calloc(nmemb,size);
	mutex_unlock(&malloc_lock);
	return p;
}

void *malloc_r(size_t size) {
	void *p;
	mutex_lock(&malloc_lock);
	p = malloc(size);
	mutex_unlock(&malloc_lock);
	return p;
}

void free_r(void *ptr) {
	mutex_lock(&malloc_lock);
	free(ptr);
	mutex_unlock(&malloc_lock);
}

void *realloc_r(void *ptr, size_t size) {
	void *p;
	mutex_lock(&malloc_lock);
	p = realloc(ptr,size);
	mutex_unlock(&malloc_lock);
	return p;
}
