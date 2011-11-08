/*
 * rts.c
 *
 *  Created on: Nov 24, 2010
 *      Author: wallento
 */

#include "malloc.h"
#include "services.h"

extern int main();

void init() __attribute__((section(".rtsinit")));

void rts_exit() {
	svc_req0(svc_identify("scheduler"),0);
}

void init() {
	main();
	rts_exit();
}

void abort() {
	rts_exit();
}

unsigned int time(unsigned int x) {
	return 123456;
}
