#include "utils.h"

#include "spr-defs.h"
#include "board.h" // For timer rate (IN_CLK, TICKS_PER_SEC)
#include "printf.h"

#include <stdlib.h>

#include "../../mutex.h"
extern mutex_t printf_mutex;

int errno;

/* For writing into SPR. */
void
mtspr(unsigned long spr, unsigned long value)
{
  asm("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}

/* For reading SPR. */
unsigned long
mfspr(unsigned long spr)
{
  unsigned long value;
  asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
  return value;
}

unsigned int coreid() {
	return mfspr(SPR_COREID);
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
arch_enable_timer(void)
{
  mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | ((IN_CLK/TICKS_PER_SEC) & SPR_TTMR_PERIOD));
  mtspr(SPR_SR, SPR_SR_TEE | SPR_SR_IEE | mfspr(SPR_SR));
}

/* Disable tick timer and interrupt generation */
void
arch_disable_timer(void)
{
  // Disable timer: clear it all!
  mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_TEE);
  mtspr(SPR_TTMR, 0);

}

void context_print(arch_thread_ctx_t *ctx) {
	printf("pc: 0x%08x, sr: 0x%08x, sp: 0x%08x\n",ctx->pc,ctx->sr,ctx->regs[0]);
	printf("  r2: 0x%08x,  r3: 0x%08x,  r4: 0x%08x,  r5: 0x%08x,  r6: 0x%08x\n", ctx->regs[1], ctx->regs[2], ctx->regs[3], ctx->regs[4], ctx->regs[5] );
	printf("  r7: 0x%08x,  r8: 0x%08x,  r9: 0x%08x, r10: 0x%08x, r11: 0x%08x\n", ctx->regs[6], ctx->regs[7], ctx->regs[8], ctx->regs[9], ctx->regs[10] );
	printf(" r12: 0x%08x, r13: 0x%08x, r14: 0x%08x, r15: 0x%08x, r16: 0x%08x\n", ctx->regs[11], ctx->regs[12], ctx->regs[13], ctx->regs[14], ctx->regs[15] );
	printf(" r17: 0x%08x, r18: 0x%08x, r19: 0x%08x, r20: 0x%08x, r21: 0x%08x\n", ctx->regs[16], ctx->regs[17], ctx->regs[18], ctx->regs[19], ctx->regs[20] );
	printf(" r22: 0x%08x, r23: 0x%08x, r24: 0x%08x, r25: 0x%08x, r26: 0x%08x\n", ctx->regs[21], ctx->regs[22], ctx->regs[23], ctx->regs[24], ctx->regs[25] );
	printf(" r27: 0x%08x, r28: 0x%08x, r29: 0x%08x, r30: 0x%08x, r31: 0x%08x\n", ctx->regs[26], ctx->regs[27], ctx->regs[28], ctx->regs[29], ctx->regs[30] );
}

unsigned int arch_context_extract_parameter(arch_thread_ctx_t *ctx,unsigned int n) {
	unsigned int val = NULL;
	if ( n < 7 ) {
		val = (unsigned int) ctx->regs[2+n];
	} else {
		// TODO: for higher numbers we need to get them from stack
	}
	return val;
}

void arch_context_set_return(arch_thread_ctx_t *ctx,unsigned int val) {
	ctx->regs[10] = (unsigned int) val;
}

void arch_multicore_init() {
	mutex_init(&printf_mutex);
}

