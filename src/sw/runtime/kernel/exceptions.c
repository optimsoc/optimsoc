#include "arch.h"

#include ARCH_INCL(printf.h)
#include ARCH_INCL(context.h)
#include ARCH_INCL(board.h)

arch_thread_ctx_t exception_ctx[NUMCORES];

#define STR_EXCEPTION_OCCURED " exception occured.\n"

char *exception_strings[] = {
  "An unknown",             // 0
  "A reset (?!)",           // 1
  "A Bus Error",            // 2
  "A Data Page Fault",      // 3
  "An Instruction Page Fault", // 4
  "A Tick-Timer",              // 5
  "An Alignment",              // 6
  "An Illegal Instruction",    // 7
  "An External Interrupt",     // 8
  "A D-TLB Miss",              // 9
  "An I-TLB Miss",             // a
  "A Range",                   // b
  "A System Call",             // c
  "A Floating-Point",          // d
  "A Trap",                    // e
  "A \"Reserved\"",            // f
  "A \"Reserved\"",            // 10
  "A \"Reserved\"",            // 11
  "A \"Reserved\"",            // 12
  "A \"Reserved\"",            // 13
  "A \"Reserved\"",            // 14
  "An Implementation Specific \"Reserved\"", // 16
  "An Implementation Specific \"Reserved\"", // 17
  "An Implementation Specific \"Reserved\"", // 18
  "A Custom",                                // 19
  "A Custom",                                // 1a
  "A Custom",                                // 1b
  "A Custom",                                // 1c
  "A Custom",                                // 1d
  "A Custom",                                // 1e
  "A Custom",                                // 1f
};

extern void int_main();

void (*except_handlers[]) (void ) = {0,    // 0
				    0,    // 1
				    0,    // 2
				    0,    // 3
				    0,    // 4
				    0,    // 5
				    0,    // 6
				    0,    // 7
				    0,    // 8
				    0,    // 9
				    0,    // a
				    0,    // b
				    0,    // c
				    0,    // d
				    0,    // e
				    0,    // f
				    0,    // 10
				    0,    // 11
				    0,    // 12
				    0};    // 13


void
add_handler(unsigned long vector, void (*handler) (void))
{
  except_handlers[vector] = handler;
}

void default_exception_handler_c(unsigned exception_address,unsigned epc, unsigned int eear)
{
  int exception_no = (exception_address >> 8) & 0x1f;
  if (except_handlers[exception_no])
    {
      return (*except_handlers[exception_no])();
    }

  printf("EPC = 0x%.8x\n", exception_address);

  // Output initial messaging using low-level functions incase
  // something really bad happened.
  printf("%s%s", exception_strings[exception_no],
	 STR_EXCEPTION_OCCURED);

  // Icing on the cake using fancy functions
  printf("EEAR = 0x%.8x\n", eear);
  printf("EPC = 0x%.8x\n", epc);
//  report(exception_address);
//  report(epc);
//  __asm__("l.nop 1");
//  for(;;);
}
