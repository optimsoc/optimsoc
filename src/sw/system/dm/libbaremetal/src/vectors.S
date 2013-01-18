#include "spr-defs.h"
#include "macros.h"

// The exception context has size of 31 GPRs+PC+SR
#define EXCEPTION_CONTEXT_SIZE 33*4
// The exception context starts at 0x4 to not use 0x0 as address
#define EXCPT_CTX_BASE 0x4
// Store the registers as also set in context.h
#define EXCPT_CTX_PC  0x04
#define EXCPT_CTX_R1  0x08
#define EXCPT_CTX_R2  0x0c
#define EXCPT_CTX_R3  0x10
#define EXCPT_CTX_R4  0x14
#define EXCPT_CTX_R5  0x18
#define EXCPT_CTX_R6  0x1c
#define EXCPT_CTX_R7  0x20
#define EXCPT_CTX_R8  0x24
#define EXCPT_CTX_R9  0x28
#define EXCPT_CTX_R10 0x2c
#define EXCPT_CTX_R11 0x30
#define EXCPT_CTX_R12 0x34
#define EXCPT_CTX_R13 0x38
#define EXCPT_CTX_R14 0x3c
#define EXCPT_CTX_R15 0x40
#define EXCPT_CTX_R16 0x44
#define EXCPT_CTX_R17 0x48
#define EXCPT_CTX_R18 0x4c
#define EXCPT_CTX_R19 0x50
#define EXCPT_CTX_R20 0x54
#define EXCPT_CTX_R21 0x58
#define EXCPT_CTX_R22 0x5c
#define EXCPT_CTX_R23 0x60
#define EXCPT_CTX_R24 0x64
#define EXCPT_CTX_R25 0x68
#define EXCPT_CTX_R26 0x6c
#define EXCPT_CTX_R27 0x70
#define EXCPT_CTX_R28 0x74
#define EXCPT_CTX_R29 0x78
#define EXCPT_CTX_R30 0x7c
#define EXCPT_CTX_R31 0x80
#define EXCPT_CTX_SR  0x84


#define EXCEPTION_HANDLER                                    \
    /* This is the initial portion of the each exception     \
     * handler. It stores the current context in the context \
     * of the exception handler, which is the memory after   \
     * address 0x4. Most of the register safe is delayed     \
     * until the call to the exception handler. Here a few   \
     * used registers are saved, the exception registers     \
     * saved, and the stack is set up. */                    \
    /* First save those five registers that we use later. */ \
    l.sw    EXCPT_CTX_R1(r0),r1;                             \
	l.sw    EXCPT_CTX_R3(r0),r3;                             \
	l.sw    EXCPT_CTX_R4(r0),r4;                             \
	l.sw    EXCPT_CTX_R5(r0),r5;                             \
	l.sw    EXCPT_CTX_R9(r0),r9;                             \
	/* We need the NPC to easily detect the exception we are \
	 * in, but it is not required to be saved */             \
	l.mfspr r3,r0,SPR_NPC;                                   \
	/* Load the exception PC as second parameter to the      \
	 * exception handler. Also store it in context. */       \
	l.mfspr r4,r0,SPR_EPCR_BASE;                             \
	l.sw    EXCPT_CTX_PC(r0),r4;                             \
	/* Store the status register in context */               \
	l.mfspr r5,r0,SPR_ESR_BASE;                              \
	l.sw    EXCPT_CTX_SR(r0),r5;                             \
	/* Set exception stack pointer. This symbol is required  \
	 * to be a pointer to the actual end of the stack. */    \
	l.movhi r1,hi(exception_stack);                          \
    l.ori   r1,r1,lo(exception_stack);                       \
    l.lwz   r1,0(r1);                                        \
    /* Jump to the default exception handler */              \
	l.jal   default_exception_handler;                       \
	l.nop;                                                   \
    l.lwz   r4,EXCPT_CTX_PC(r0);                             \
    l.mtspr r0,r4,SPR_EPCR_BASE;                             \
    l.lwz   r3,EXCPT_CTX_SR(r0);                             \
    l.mtspr r0,r3,SPR_ESR_BASE;                              \
    l.lwz   r9,EXCPT_CTX_R9(r0);                             \
    l.lwz   r5,EXCPT_CTX_R5(r0);                             \
    l.lwz   r4,EXCPT_CTX_R4(r0);                             \
    l.lwz   r3,EXCPT_CTX_R3(r0);                             \
    l.lwz   r1,EXCPT_CTX_R1(r0);                             \
	l.rfe;                                                   \
	l.nop;


/* =================================================== [ exceptions ] === */
	.section .vectors, "ax"

/* ---[ 0x100: RESET exception ]----------------------------------------- */
        .org 0x100
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
	EXCEPTION_HANDLER

/* ---[ 0x300: Data Page Fault exception ]------------------------------- */
	.org 0x300
	EXCEPTION_HANDLER

/* ---[ 0x400: Insn Page Fault exception ]------------------------------- */
	.org 0x400
	EXCEPTION_HANDLER


/* ---[ 0x500: Timer exception ]----------------------------------------- */
	.org 0x500
	EXCEPTION_HANDLER

/* ---[ 0x600: Aligment exception ]-------------------------------------- */
	.org 0x600
	EXCEPTION_HANDLER

/* ---[ 0x700: Illegal insn exception ]---------------------------------- */
	.org 0x700
	EXCEPTION_HANDLER

/* ---[ 0x800: External interrupt exception ]---------------------------- */
	.org 0x800
	EXCEPTION_HANDLER

/* ---[ 0x900: DTLB miss exception ]------------------------------------- */
	.org 0x900
	EXCEPTION_HANDLER

/* ---[ 0xa00: ITLB miss exception ]------------------------------------- */
	.org 0xa00
	EXCEPTION_HANDLER


/* ---[ 0xb00: Range exception ]----------------------------------------- */
	.org 0xb00
	EXCEPTION_HANDLER


/* ---[ 0xc00: Syscall exception ]--------------------------------------- */
	.org 0xc00
	EXCEPTION_HANDLER

/* ---[ 0xd00: Trap exception ]------------------------------------------ */
	.org 0xd00
	EXCEPTION_HANDLER


/* ---[ 0xe00: Trap exception ]------------------------------------------ */
	.org 0xe00
	EXCEPTION_HANDLER


/* ---[ 0xf00: Reserved exceptions ]------------------------------------- */
	.org 0xf00

/* ====================================== [ default exception handler ] === */

default_exception_handler:
    l.sw    0xfffffffc(r1),r9
	l.sw    EXCPT_CTX_R2(r0),r2
	l.sw    EXCPT_CTX_R6(r0),r6
	l.sw    EXCPT_CTX_R7(r0),r7
	l.sw    EXCPT_CTX_R8(r0),r8
	l.sw    EXCPT_CTX_R10(r0),r10
	l.sw    EXCPT_CTX_R11(r0),r11
	l.sw    EXCPT_CTX_R12(r0),r12
	l.sw    EXCPT_CTX_R13(r0),r13
	l.sw    EXCPT_CTX_R14(r0),r14
	l.sw    EXCPT_CTX_R15(r0),r15
	l.sw    EXCPT_CTX_R16(r0),r16
	l.sw    EXCPT_CTX_R17(r0),r17
	l.sw    EXCPT_CTX_R18(r0),r18
	l.sw    EXCPT_CTX_R19(r0),r19
	l.sw    EXCPT_CTX_R20(r0),r20
	l.sw    EXCPT_CTX_R21(r0),r21
	l.sw    EXCPT_CTX_R22(r0),r22
	l.sw    EXCPT_CTX_R23(r0),r23
	l.sw    EXCPT_CTX_R24(r0),r24
	l.sw    EXCPT_CTX_R25(r0),r25
	l.sw    EXCPT_CTX_R26(r0),r26
	l.sw    EXCPT_CTX_R27(r0),r27
	l.sw    EXCPT_CTX_R28(r0),r28
	l.sw    EXCPT_CTX_R29(r0),r29
	l.sw    EXCPT_CTX_R30(r0),r30
	l.sw    EXCPT_CTX_R31(r0),r31

	l.jal   default_exception_handler_c
	l.addi  r1,r1,0xfffffffc

	l.addi  r1,r1,4
    l.lwz   r9,0xfffffffc(r1)
	// Register 3,4,5 and 9 are restored by the calling exception
    l.lwz   r2,EXCPT_CTX_R2(r0)
    l.lwz   r6,EXCPT_CTX_R6(r0)
    l.lwz   r7,EXCPT_CTX_R7(r0)
    l.lwz   r8,EXCPT_CTX_R8(r0)
    l.lwz   r10,EXCPT_CTX_R10(r0)
    l.lwz   r11,EXCPT_CTX_R11(r0)
    l.lwz   r12,EXCPT_CTX_R12(r0)
    l.lwz   r13,EXCPT_CTX_R13(r0)
    l.lwz   r14,EXCPT_CTX_R14(r0)
    l.lwz   r15,EXCPT_CTX_R15(r0)
    l.lwz   r16,EXCPT_CTX_R16(r0)
    l.lwz   r17,EXCPT_CTX_R17(r0)
    l.lwz   r18,EXCPT_CTX_R18(r0)
    l.lwz   r19,EXCPT_CTX_R19(r0)
    l.lwz   r20,EXCPT_CTX_R20(r0)
    l.lwz   r21,EXCPT_CTX_R21(r0)
    l.lwz   r22,EXCPT_CTX_R22(r0)
    l.lwz   r23,EXCPT_CTX_R23(r0)
    l.lwz   r24,EXCPT_CTX_R24(r0)
    l.lwz   r25,EXCPT_CTX_R25(r0)
    l.lwz   r26,EXCPT_CTX_R26(r0)
    l.lwz   r27,EXCPT_CTX_R27(r0)
    l.lwz   r28,EXCPT_CTX_R28(r0)
    l.lwz   r29,EXCPT_CTX_R29(r0)
    l.lwz   r30,EXCPT_CTX_R30(r0)
    l.lwz   r31,EXCPT_CTX_R31(r0)
	l.jr    r9
	l.nop
