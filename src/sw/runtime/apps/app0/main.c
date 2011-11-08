/*
 * main.c
 *
 *  Created on: Nov 23, 2010
 *      Author: wallento
 */

#include <librts.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	dma_handler *dma;
	printf("Application 0 starts\n");

	printf("Start dma\n");
	dma = dma_transfer(NULL,NULL,100000);

	dma_wait(dma);
	printf("dma done\n");

	for (unsigned int i=0; i < 100000; ++i ) __asm__("l.nop 200");
	printf("Application 0 ends\n");
}
