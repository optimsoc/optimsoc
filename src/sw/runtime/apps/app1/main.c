/*
 * main.c
 *
 *  Created on: Nov 23, 2010
 *      Author: wallento
 */

#include <librts.h>
#include <stdlib.h>
#include <stdio.h>

int test() {
	void *ptr = malloc(4);
	return (unsigned int) ptr;
}

int main(int argc, char *argv[]) {
	printf("Application 1 starts\n");
	test();
	for (unsigned int i=0; i < 100000; ++i ) __asm__("l.nop 200");
	printf("Application 1 ends\n");
}
