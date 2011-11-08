/*
 * malloc.h
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#ifndef MALLOC_H_
#define MALLOC_H_

#define malloc  malloc_r
#define calloc  calloc_r
#define free    free_r
#define realloc realloc_r

void *calloc_r(size_t nmemb, size_t size);
void *malloc_r(size_t size);
void free_r(void *ptr);
void *realloc_r(void *ptr, size_t size);

#endif /* MALLOC_H_ */
