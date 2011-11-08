#include "context-defs.h"
#include "spr-defs.h"
#include "board.h"
#include "macros.h"

#define EXCEPTION_HANDLER_START     		\
	l.mfspr	r31, r0, SPR_COREID;			\
	l.slli	r31, r31, 3;					\
	l.addi	r31, r31, 0x10;					\
	l.sw	0(r31), r1;						\
	l.sw	4(r31), r2;						\
	LOAD_SYMBOL_2_GPR(r1,exception_ctx);	\
	l.mfspr r2, r0, SPR_COREID;				\
	l.muli	r2, r2, CONTEXT_SIZE;			\
	l.add	r1, r1, r2;						\
	l.sw	GPR9(r1), r9;					\
	l.lwz	r9, 0(r31);						\
	l.sw	SP(r1),r9;						\
	l.lwz	r2, 4(r31);						\
    l.mfspr r9,r0,SPR_EPCR_BASE;			\
    l.sw    PC(r1),r9;						\
    l.mfspr r9,r0,SPR_ESR_BASE;				\
    l.sw    SR(r1),r9;					    \
	l.jal	save_reg2to31ex9_to_context;	\
	l.nop;									\
	l.add 	r9, r0, r1;						\
	l.mfspr r2, r0, SPR_COREID;				\
	l.slli	r2, r2, 2;						\
	LOAD_SYMBOL_2_GPR(r1,kernel_sp);		\
	l.add 	r1, r1, r2;						\
	l.lwz	r1,0(r1);                       \

#define EXCEPTION_HANDLER_END		        \
	LOAD_SYMBOL_2_GPR(r1,exception_ctx);	\
	l.mfspr r2, r0, SPR_COREID;				\
	l.muli	r2, r2, CONTEXT_SIZE;			\
	l.add	r1, r1, r2;						\
	l.j restore_from_context;				\
	l.nop


#define UNHANDLED_EXCEPTION					\
	EXCEPTION_HANDLER_START					\
	l.mfspr r3,r0,SPR_NPC;          		\
	l.mfspr r4,r0,SPR_EPCR_BASE;            \
    l.mfspr r5,r0,SPR_EEAR_BASE;			\
	l.jal   default_exception_handler_c;	\
	l.nop;									\
	EXCEPTION_HANDLER_END

/* =================================================== [ exceptions ] === */
	.section .vectors, "ax"

/* ---[ 0x100: RESET exception ]----------------------------------------- */
        .org 0x100
        /* Initialize SMP-barrier at 0x10 */
        l.addi  r1,r0,0x0
        l.sw	0x10(r0),r1
        LOAD_SYMBOL_2_GPR(r3,multicore_boot_barrier)
		l.ori   r4,r0,1
		l.sw    0(r3),r4
        /* go on */
        l.addi  r1,r0,0x0
        l.addi  r2,r0,0x0
        l.addi  r3,r0,0x0
        l.addi  r4,r0,0x0
        l.addi  r5,r0,0x0
        l.addi  r6,r0,0x0
        l.addi  r7,r0,0x0
        l.addi  r8,r0,0x0
        l.addi  r9,r0,0x0
        l.addi  r10,r0,0x0
        l.addi  r11,r0,0x0
        l.addi  r12,r0,0x0
        l.addi  r13,r0,0x0
        l.addi  r14,r0,0x0
        l.addi  r15,r0,0x0
        l.addi  r16,r0,0x0
        l.addi  r17,r0,0x0
        l.addi  r18,r0,0x0
        l.addi  r19,r0,0x0
        l.addi  r20,r0,0x0
        l.addi  r21,r0,0x0
        l.addi  r22,r0,0x0
        l.addi  r23,r0,0x0
        l.addi  r24,r0,0x0
        l.addi  r25,r0,0x0
        l.addi  r26,r0,0x0
        l.addi  r27,r0,0x0
        l.addi  r28,r0,0x0
        l.addi  r29,r0,0x0
        l.addi  r30,r0,0x0
        l.addi  r31,r0,0x0
	/* Clear status register, set supervisor mode */
	l.ori r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer  */
	l.mtspr r0, r0, SPR_TTMR

	/* Jump to program initialisation code */
	LOAD_SYMBOL_2_GPR(r4, _start)
	l.jr    r4
	l.nop

/* ---[ 0x200: BUS exception ]------------------------------------------- */
	.org 0x200
	UNHANDLED_EXCEPTION

/* ---[ 0x300: Data Page Fault exception ]------------------------------- */
	.org 0x300
	UNHANDLED_EXCEPTION

/* ---[ 0x400: Insn Page Fault exception ]------------------------------- */
	.org 0x400
	UNHANDLED_EXCEPTION


/* ---[ 0x500: Timer exception ]----------------------------------------- */
	.org 0x500
	EXCEPTION_HANDLER_START
	l.nop;
	l.add	r3,r0,r9;
	l.jal	scheduler_timer;
	l.nop
	EXCEPTION_HANDLER_END

/* ---[ 0x600: Aligment exception ]-------------------------------------- */
	.org 0x600
	UNHANDLED_EXCEPTION

/* ---[ 0x700: Illegal insn exception ]---------------------------------- */
	.org 0x700
	UNHANDLED_EXCEPTION

/* ---[ 0x800: External interrupt exception ]---------------------------- */
	.org 0x800
	EXCEPTION_HANDLER_START
	l.jal   interrupt;
	l.nop;
	EXCEPTION_HANDLER_END

/* ---[ 0x900: DTLB miss exception ]------------------------------------- */
	.org 0x900
	EXCEPTION_HANDLER_START
	l.mfspr r3,r0,SPR_EEAR_BASE;
	l.mfspr r4,r0,SPR_EPCR_BASE;
	l.jal   dtlb_miss_handler;
	l.nop;
	EXCEPTION_HANDLER_END

/* ---[ 0xa00: ITLB miss exception ]------------------------------------- */
	.org 0xa00
	EXCEPTION_HANDLER_START
	l.mfspr r3,r0,SPR_EEAR_BASE;
	l.mfspr r4,r0,SPR_EPCR_BASE;
	l.jal   itlb_miss_handler;
	l.nop;
	EXCEPTION_HANDLER_END


/* ---[ 0xb00: Range exception ]----------------------------------------- */
	.org 0xb00
	UNHANDLED_EXCEPTION


/* ---[ 0xc00: Syscall exception ]--------------------------------------- */
	.org 0xc00
	EXCEPTION_HANDLER_START
	l.add	r3,r0,r9;
	l.add	r4,r0,r11;
	l.jal	syscall;
	l.nop
	EXCEPTION_HANDLER_END

/* ---[ 0xd00: Trap exception ]------------------------------------------ */
	.org 0xd00
	UNHANDLED_EXCEPTION


/* ---[ 0xe00: Trap exception ]------------------------------------------ */
	.org 0xe00
	UNHANDLED_EXCEPTION


/* ---[ 0xf00: Reserved exceptions ]------------------------------------- */
	.org 0xf00
	UNHANDLED_EXCEPTION

/* ====================================== [ default exception handler ] === */

unhandled_exception:
	UNHANDLED_EXCEPTION

save_reg2to31ex9_to_context:
	l.sw	GPR2(r1), r2;
	l.sw	GPR3(r1), r3;
	l.sw	GPR4(r1), r4;
	l.sw	GPR5(r1), r5;
	l.sw	GPR6(r1), r6;
	l.sw	GPR7(r1), r7;
	l.sw	GPR8(r1), r8;
	l.sw	GPR10(r1), r10;
	l.sw	GPR11(r1), r11;
	l.sw	GPR12(r1), r12;
	l.sw	GPR13(r1), r13;
	l.sw	GPR14(r1), r14;
	l.sw	GPR15(r1), r15;
	l.sw	GPR16(r1), r16;
	l.sw	GPR17(r1), r17;
	l.sw	GPR18(r1), r18;
	l.sw	GPR19(r1), r19;
	l.sw	GPR20(r1), r20;
	l.sw	GPR21(r1), r21;
	l.sw	GPR22(r1), r22;
	l.sw	GPR23(r1), r23;
	l.sw	GPR24(r1), r24;
	l.sw	GPR25(r1), r25;
	l.sw	GPR26(r1), r26;
	l.sw	GPR27(r1), r27;
	l.sw	GPR28(r1), r28;
	l.sw	GPR29(r1), r29;
	l.sw	GPR30(r1), r30;
	l.sw	GPR31(r1), r31;
	l.jr	r9;
	l.nop

restore_from_context:
	l.lwz   r3,PC(r1);
	l.mtspr r0,r3,SPR_EPCR_BASE;
	l.lwz   r3,SR(r1);
	l.mtspr r0,r3,SPR_ESR_BASE;
	l.lwz	r2, GPR2(r1);
	l.lwz	r3, GPR3(r1);
	l.lwz	r4, GPR4(r1);
	l.lwz	r5, GPR5(r1);
	l.lwz	r6, GPR6(r1);
	l.lwz	r7, GPR7(r1);
	l.lwz	r8, GPR8(r1);
	l.lwz	r9, GPR9(r1);
	l.lwz	r10, GPR10(r1);
	l.lwz	r11, GPR11(r1);
	l.lwz	r12, GPR12(r1);
	l.lwz	r13, GPR13(r1);
	l.lwz	r14, GPR14(r1);
	l.lwz	r15, GPR15(r1);
	l.lwz	r16, GPR16(r1);
	l.lwz	r17, GPR17(r1);
	l.lwz	r18, GPR18(r1);
	l.lwz	r19, GPR19(r1);
	l.lwz	r20, GPR20(r1);
	l.lwz	r21, GPR21(r1);
	l.lwz	r22, GPR22(r1);
	l.lwz	r23, GPR23(r1);
	l.lwz	r24, GPR24(r1);
	l.lwz	r25, GPR25(r1);
	l.lwz	r26, GPR26(r1);
	l.lwz	r27, GPR27(r1);
	l.lwz	r28, GPR28(r1);
	l.lwz	r29, GPR29(r1);
	l.lwz	r30, GPR30(r1);
	l.lwz	r31, GPR31(r1);
	l.lwz	r1, SP(r1);
	l.rfe;
	l.nop
