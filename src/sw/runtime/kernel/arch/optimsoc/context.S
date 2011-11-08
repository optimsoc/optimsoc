#include "context-defs.h"
#include "spr-defs.h"

.section .text

.global context_exception_store_all;
.type   context_exception_store_all,function;
.align;

context_exception_store_all:
	l.sw    GPR3(r1),r3
    l.mfspr r3,r0,SPR_EPCR_BASE
    l.sw    PC(r1),r3
    l.mfspr r3,r0,SPR_ESR_BASE
    l.sw    SR(r1),r3
    l.addi  r3,r1,+(CONTEXT_SIZE) /* original SP when the exception occured */
    l.sw    SP(r1),r3
    l.sw    GPR2(r1),r2
	l.lwz   r3,GPR3(r1)
	l.sw    GPR4(r1),r4
    l.sw    GPR5(r1),r5
    l.sw    GPR6(r1),r6
    l.sw    GPR7(r1),r7
    l.sw    GPR8(r1),r8
	l.sw    GPR10(r1),r10
    l.sw    GPR11(r1),r11
    l.sw    GPR12(r1),r12
    l.sw    GPR13(r1),r13
    l.sw    GPR14(r1),r14
    l.sw    GPR15(r1),r15
    l.sw    GPR16(r1),r16
    l.sw    GPR17(r1),r17
    l.sw    GPR18(r1),r18
    l.sw    GPR19(r1),r19
    l.sw    GPR20(r1),r20
    l.sw    GPR21(r1),r21
    l.sw    GPR22(r1),r22
    l.sw    GPR23(r1),r23
    l.sw    GPR24(r1),r24
    l.sw    GPR25(r1),r25
    l.sw    GPR26(r1),r26
    l.sw    GPR27(r1),r27
    l.sw    GPR28(r1),r28
    l.sw    GPR29(r1),r29
    l.sw    GPR30(r1),r30
    l.sw    GPR31(r1),r31
	l.jr	r9
	l.nop
.fend_context_exception_store_all:
.size  context_exception_store_all,.fend_context_exception_store_all-context_exception_store_all;

.global context_exception_restore_all;
.type   context_exception_restore_all,function;
.align;
context_exception_restore_all:
	l.lwz   r3,PC(r1)
	l.mtspr r0,r3,SPR_EPCR_BASE
	l.lwz   r3,SR(r1)
	l.mtspr r0,r3,SPR_ESR_BASE
	l.lwz   r2,GPR2(r1)
	l.lwz   r3,GPR3(r1)
	l.lwz   r4,GPR4(r1)
	l.lwz   r5,GPR5(r1)
	l.lwz   r6,GPR6(r1)
	l.lwz   r7,GPR7(r1)
	l.lwz   r8,GPR8(r1)
	l.lwz   r9,GPR9(r1)
	l.lwz   r10,GPR10(r1)
	l.lwz   r11,GPR11(r1)
	l.lwz   r12,GPR12(r1)
	l.lwz   r13,GPR13(r1)
	l.lwz   r14,GPR14(r1)
	l.lwz   r15,GPR15(r1)
	l.lwz   r16,GPR16(r1)
	l.lwz   r17,GPR17(r1)
	l.lwz   r18,GPR18(r1)
	l.lwz   r19,GPR19(r1)
	l.lwz   r20,GPR20(r1)
	l.lwz   r21,GPR21(r1)
	l.lwz   r22,GPR22(r1)
	l.lwz   r23,GPR23(r1)
	l.lwz   r24,GPR24(r1)
	l.lwz   r25,GPR25(r1)
	l.lwz   r26,GPR26(r1)
	l.lwz   r27,GPR27(r1)
	l.lwz   r28,GPR28(r1)
	l.lwz   r29,GPR29(r1)
	l.lwz   r30,GPR30(r1)
	l.lwz   r31,GPR31(r1)
	l.lwz   r1,SP(r1)
	l.rfe
	l.nop
.fend_context_exception_restore_all:
.size  context_exception_restore_all,.fend_context_exception_restore_all-context_exception_restore_all;

.global arch_context_switch;
.type   arch_context_switch,function;
.align;

arch_context_switch:
	l.addi 	r1,r1,8
	l.add 	r11,r0,r1				/* backup current r1 into r11 */
	l.add 	r1,r0,r3				/* get the *old into r1 as the sp to push the context to */

	l.addi  r1,r1,-(CONTEXT_SIZE);
    l.sw    GPR9(r1),r9;
    l.jal	context_exception_store_all;
	l.nop

	l.sw 	SP(r1),r11	/* store the backed up r1 earlier as the correct SP in the saved context */
	l.add 	r1,r0,r4

//	l.lwz 	r11,GPR11(r1);
	l.j		context_exception_restore_all;
	l.nop


.fend_arch_context_switch:
.size  arch_context_switch,.fend_arch_context_switch-arch_context_switch;


