#include "utils.h"
#include <stdlib.h>

#define N 2048
#define START 0x0010000
#define END   0x7800000

unsigned int data[N];

void set_data(unsigned int base) {
  for (int i=0;i<N;i++) {
   data[i] = 0+i;
  }
}

void copy_data(unsigned int base) {
  REG32(0xe0201040) = &data[0]; // Local address
  REG32(0xe0201044) = N; // Size in words
  REG32(0xe0201048) = 1; // Remote tile
  REG32(0xe020104c) = base; // Remote address
  REG32(0xe0201050) = 0; // R2L
  REG32(0xe0201054) = 1; // valid
  while (REG32(0xe0201054)==0) {}
}

void readback_data(unsigned int base) {
  REG32(0xe0201040) = &data[0]; // Local address
  REG32(0xe0201044) = N; // Size in words
  REG32(0xe0201048) = 1; // Remote tile
  REG32(0xe020104c) = base; // Remote address
  REG32(0xe0201050) = 1; // R2L
  REG32(0xe0201054) = 1; // valid
  while (REG32(0xe0201054)==0) {}
}

void verify_data(unsigned int base) {
	int errors = 0;
	for (int i=0;i<N;i++) {
		if (data[i] != i) {
			errors++;
			OPTIMSOC_REPORT(0x203,i)
		}
	}
	if (errors>0) {
		OPTIMSOC_REPORT(0x204,errors)
	}
}

void main() {
  void *p = malloc(4);
  //OPTIMSOC_REPORT(0x200,(unsigned int)p)
  if (REG32(0xe0000000)==0) {
	  for (int b=START;b<END;b+=0x2000) {
		  set_data(b);
		  copy_data(b);
		  readback_data(b);
		  verify_data(b);
		  OPTIMSOC_REPORT(0x200,b)
	  }
  }
}
