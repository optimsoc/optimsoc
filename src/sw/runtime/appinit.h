/*
 * appinit.h
 *
 *  Created on: Nov 28, 2010
 *      Author: wallento
 */

#ifndef APPINIT_H_
#define APPINIT_H_

typedef struct app_t {
	void *start;
	void *end;
} app_t;

extern app_t        app_init[];
extern unsigned int app_init_size;

#endif /* APPINIT_H_ */
