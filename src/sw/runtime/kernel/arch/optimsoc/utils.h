#ifndef _OR1200_UTILS_H_
#define _OR1200_UTILS_H_

#include "context.h"

/* Register access macros */
#define REG8(add) *((volatile unsigned char *)(add))
#define REG16(add) *((volatile unsigned short *)(add))
#define REG32(add) *((volatile unsigned long *)(add))

/*
 * l.nop constants
 *
 */
#define NOP_NOP         0x0000      /* Normal nop instruction */
#define NOP_EXIT        0x0001      /* End of simulation */
#define NOP_REPORT      0x0002      /* Simple report */
#define NOP_PRINTF      0x0003      /* Simprintf instruction */
#define NOP_PUTC        0x0004      /* Simulation putc instruction */
#define NOP_REPORT_FIRST 0x0400     /* Report with number */
#define NOP_REPORT_LAST  0x03ff      /* Report with number */

#define NOP_REPORT_SCHEDULED_THREAD 0x0030     /* Report with number */

/* For writing into SPR. */
void mtspr(unsigned long spr, unsigned long value);

/* For reading SPR. */
unsigned long mfspr(unsigned long spr);

/* Print out a character via simulator */
void sim_putc(unsigned char c);

/* Prints out a value */
void report(unsigned long value);

/* Loops/exits simulation */
void or32exit(int i);

/* Enable user interrupts */
void cpu_enable_user_interrupts(void);

unsigned int coreid();

/* Enable tick timer and interrupt generation */
void arch_enable_timer(void);
/* Disable tick timer and interrupt generation */
void arch_disable_timer(void);

void context_print(arch_thread_ctx_t *ctx);

void arch_multicore_init();

inline void
arch_report(unsigned long value)
{
  asm volatile ("l.addi\tr19,%0,0": :"r" (value));
  asm volatile ("l.nop %0": :"K" (NOP_REPORT));
}

inline void arch_report_scheduled_thread(unsigned long id) {
  asm volatile ("l.addi\tr19,%0,0": :"r" (id));
  asm volatile ("l.nop %0": :"K" (NOP_REPORT_SCHEDULED_THREAD));
}

#define MAPCO_LLSC_BASE 0xef000000
#define MAPCO_LL_SETUP   MAPCO_LLSC_BASE+0x0
#define MAPCO_SC_RESULT  MAPCO_LLSC_BASE+0x4
#define MAPCO_LLSC_ABORT MAPCO_LLSC_BASE+0x8

inline void* arch_load_linked(void* address) {
	void* value;
	asm volatile ("l.sw 0(%[llsetup]),%[address]\nl.lwz %[value],0(%[llsetup])\n" : [value] "=r" (value) : [address] "r" (address), [llsetup] "r" (MAPCO_LL_SETUP) );
	return value;
}

inline char arch_store_conditional(void* address,void* value) {
	char result;
	asm volatile ("l.sw 0(%[address]),%[value]\nl.lbz %[result],0(%[scresult])" : [result] "=r" (result) : [address] "r" (address), [value] "r" (value), [scresult] "r" (MAPCO_SC_RESULT) );
	return result;
}

inline void arch_llsc_abort() {
	asm volatile ("l.sw 0(%[llscabort]),1" : : [llscabort] "r" (MAPCO_LLSC_ABORT));
}

#define NA_BASE   0x80000000
#define NA_TILEID NA_BASE + 0x0

inline unsigned int arch_get_tileid() {
	return *((volatile unsigned int*)NA_TILEID);
}

#endif
