#include <spr-defs.h>

.section .text

.global ctx_replace;
.type   ctx_replace,function;
.align;

ctx_replace:
	l.lwz   r1,0x04(r0)
	l.mtspr r0,r1,SPR_EPCR_BASE;
	l.lwz   r1,0x84(r0)
	l.mtspr r0,r1,SPR_ESR_BASE;
    l.lwz   r1,0x08(r0)
    l.lwz   r2,0x0c(r0)
    l.lwz   r3,0x10(r0)
    l.lwz   r4,0x14(r0)
    l.lwz   r5,0x18(r0)
    l.lwz   r6,0x1c(r0)
    l.lwz   r7,0x20(r0)
    l.lwz   r8,0x24(r0)
    l.lwz   r9,0x28(r0)
    l.lwz   r10,0x2c(r0)
    l.lwz   r11,0x30(r0)
    l.lwz   r12,0x34(r0)
    l.lwz   r13,0x38(r0)
    l.lwz   r14,0x3c(r0)
    l.lwz   r15,0x40(r0)
    l.lwz   r16,0x44(r0)
    l.lwz   r17,0x48(r0)
    l.lwz   r18,0x4c(r0)
    l.lwz   r19,0x50(r0)
    l.lwz   r20,0x54(r0)
    l.lwz   r21,0x58(r0)
    l.lwz   r22,0x5c(r0)
    l.lwz   r23,0x60(r0)
    l.lwz   r24,0x64(r0)
    l.lwz   r25,0x68(r0)
    l.lwz   r26,0x6c(r0)
    l.lwz   r27,0x70(r0)
    l.lwz   r28,0x74(r0)
    l.lwz   r29,0x78(r0)
    l.lwz   r30,0x7c(r0)
    l.lwz   r31,0x80(r0)
    l.rfe
    l.nop

.fend_ctx_replace:
.size  ctx_replace,.fend_ctx_replace-ctx_replace;
