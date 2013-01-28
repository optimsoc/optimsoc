#ifndef _OR1200_UTILS_H_
#define _OR1200_UTILS_H_

#include "spr-defs.h"

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

#define OPTIMSOC_REPORT(id,v)        \
   asm("l.addi\tr3,%0,0": :"r" (v) : "r3"); \
   asm("l.nop %0": :"K" (id));

#define OPTIMSOC_NOP_STATUS 0x0100
#define OPTIMSOC_NOP_STATUS_BOOTED 1

static inline void optimsoc_report_booted() {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_STATUS,OPTIMSOC_NOP_STATUS_BOOTED);
}

inline void
mtspr(unsigned long spr, unsigned long value)
{
  asm("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}

/* For reading SPR. */
inline unsigned long
mfspr(unsigned long spr)
{
  unsigned long value;
  asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
  return value;
}

/* Print out a character via simulator */
void sim_putc(unsigned char c);

/* Prints out a value */
void report(unsigned long value);

/* Loops/exits simulation */
void or32exit(int i);

/* Enable user interrupts */
void cpu_enable_user_interrupts(void);

/* Enable tick timer and interrupt generation */
extern void enable_timer(void);
/* Disable tick timer and interrupt generation */
extern void disable_timer(void);

void arch_disable_irq_dma();
void arch_enable_interrupts();
void arch_disable_interrupts();

inline void arch_enable_dcache(unsigned int cache_state) {
  mtspr(SPR_SR,mfspr(SPR_SR) | cache_state);
}

inline void arch_disable_dcache(unsigned int *cache_state) {
  *cache_state = mfspr(SPR_SR) & SPR_SR_DCE;
  if (*cache_state)
    mtspr(SPR_SR,mfspr(SPR_SR) & ~SPR_SR_DCE);
}

static inline int extract_bits(unsigned int x, unsigned int msb,unsigned int lsb) {
	return ((x>>lsb) & ~(~0 << (msb-lsb+1)));
}

static inline void set_bits(unsigned int *x,unsigned int v,unsigned int msb,unsigned int lsb) {
	*x = (((~0 << (msb+1) | ~(~0 << lsb))&(*x)) | ((v & ~(~0<<(msb-lsb+1))) << lsb));
}

#endif
