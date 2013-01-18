// Start function that initializes the processor
// Most of this code is from ORPSoC

#include "macros.h"
#include "spr-defs.h"

/* ========================================================= [ entry ] === */
	.section .bootload

.global _start;
.type   _start,function;
.align;

ENTRY(_start)
	/* Instruction cache enable */
	/* Check if IC present and skip enabling otherwise */
	l.mfspr r24,r0,SPR_UPR
	l.andi  r26,r24,SPR_UPR_ICP
	l.sfeq  r26,r0
	l.bf    .L8
	l.nop
//#ifndef SIMULATION
	/* Disable IC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_ICE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR

	/* Establish cache block size
	If BS=0, 16;
	If BS=1, 32;
	r14 contain block size
	*/
	l.mfspr r24,r0,SPR_ICCFGR
	l.andi  r26,r24,SPR_ICCFGR_CBS
	l.srli  r28,r26,7
	l.ori   r30,r0,16
	l.sll   r14,r30,r28

	/* Establish number of cache sets
	r16 contains number of cache sets
	r28 contains log(# of cache sets)
	*/
	l.andi  r26,r24,SPR_ICCFGR_NCS
	l.srli  r28,r26,3
	l.ori   r30,r0,1
	l.sll   r16,r30,r28

	/* Invalidate IC */
	l.addi  r6,r0,0
	l.sll   r5,r14,r28

.L7:
	l.mtspr r0,r6,SPR_ICBIR
	l.sfne  r6,r5
	l.bf    .L7
	l.add   r6,r6,r14
//#endif
	/* Enable IC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_ICE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop

.L8:
	/* Data cache enable */
    /* Check if DC present and skip enabling otherwise */
	l.mfspr r24,r0,SPR_UPR
	l.andi  r26,r24,SPR_UPR_DCP
	l.sfeq  r26,r0
	l.bf    .L10
	l.nop
//#ifndef SIMULATION
	/* Disable DC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_DCE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	/* Establish cache block size
	   If BS=0, 16;
       If BS=1, 32;
       r14 contain block size
	*/
	l.mfspr r24,r0,SPR_DCCFGR
	l.andi  r26,r24,SPR_DCCFGR_CBS
	l.srli  r28,r26,7
	l.ori   r30,r0,16
	l.sll   r14,r30,r28
	/* Establish number of cache sets
       r16 contains number of cache sets
       r28 contains log(# of cache sets)
	*/
	l.andi  r26,r24,SPR_DCCFGR_NCS
	l.srli  r28,r26,3
	l.ori   r30,r0,1
	l.sll   r16,r30,r28
	/* Invalidate DC */
	l.addi  r6,r0,0
	l.sll   r5,r14,r28
.L9:
	l.mtspr r0,r6,SPR_DCBIR
	l.sfne  r6,r5
	l.bf    .L9
	l.add   r6,r6,r14
//#endif
	/* Enable DC */
	// don't enable for the moment
//	l.mfspr r6,r0,SPR_SR
//	l.ori   r6,r6,SPR_SR_DCE
//	l.mtspr r0,r6,SPR_SR

.L10:
	/* Initialise stack */
	/* Set stack pointer */
    l.movhi r1,hi(_stack)
    l.ori   r1,r1,lo(_stack)

//#ifndef SIMULATION
    /* Clear BSS */
	LOAD_SYMBOL_2_GPR(r28, ___bss_start)
	LOAD_SYMBOL_2_GPR(r30, ___bss_end)
.bssloop:
	l.sw    (0)(r28), r0
	l.sfltu r28, r30
	l.bf    .bssloop
	l.addi  r28, r28, 4

	l.addi	r28, r0, 1
	l.sw	0x10(r0), r28

.skipbss:
	/* Enable interrupts */
	l.mfspr r3,r0,SPR_SR
	l.ori   r3,r3,SPR_SR_IEE
	l.mtspr r0,r3,SPR_SR
//#endif
	/* Jump to main program entry point (argc = argv = 0) */
	CLEAR_GPR(r3)
	CLEAR_GPR(r4)
	l.jal   _optimsoc_dm_boot
	l.nop

	l.nop  0x1
exit:
	l.j	   exit
	l.nop  0x0

.size   _start, .-_start
