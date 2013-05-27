#include <mp_simple.h>
#include <optimsoc.h>

int main() {
  OPTIMSOC_REPORT(0x10,0);
  
  unsigned int *cdc = 0xe0000208;
  *cdc = 1;
  for(int i=0;i<10;i=i+1) { asm("l.nop"); }
//  *cdc = 2;  
  for(int i=0;i<10;i=i+1) { asm("l.nop"); }
//  *cdc = 2;  
}
