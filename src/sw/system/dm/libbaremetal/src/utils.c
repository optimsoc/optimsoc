// Utility functions
// Most parts of these functions are from the ORPSoC repository

#include "utils.h"

#include "spr-defs.h"
#include <stdio.h>
#include "mutex.h"
#include "sysconfig.h"

#include <stdlib.h>

int errno;

unsigned int coreid() {
	return mfspr(SPR_COREID);
}

unsigned int tileid() {
  return *((unsigned int*) 0xe0000000);
}

/* Print out a character via simulator */
void
sim_putc(unsigned char c)
{
  asm("l.addi\tr3,%0,0": :"r" (c));
  asm("l.nop %0": :"K" (NOP_PUTC));
}

/* Loops/exits simulation */
void
or32exit (int i)
{
  asm("l.add r3,r0,%0": : "r" (i));
  asm("l.nop %0": :"K" (NOP_EXIT));
  exit(i);
}

/* Enable user interrupts */
void
arch_enable_user_interrupts(void)
{
  /* Enable interrupts in supervisor register */
  mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
}

/* Tick timer functions */
/* Enable tick timer and interrupt generation */
void
enable_timer(void)
{
  mtspr(SPR_TTCR,0);
  mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_SR | ((optimsoc_system_clock/optimsoc_ticks) & SPR_TTMR_PERIOD));
  mtspr(SPR_SR, SPR_SR_TEE | mfspr(SPR_SR));
}

/* Disable tick timer and interrupt generation */
void
disable_timer(void)
{
  // Disable timer: clear it all!
  mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_TEE);
  mtspr(SPR_TTMR, 0);

}
