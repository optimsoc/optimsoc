#define GPR_BUF_OFFSET(x) (x << 2)

syscall_entry:
	// Load address of syscall data structures

	// Add core offset -> r3

	// Store id
	l.lwz	r4,GPR_BUF_OFFSET(3)(r1)
	l.sw	0(r3),r4
	// Store parameters
	l.lwz	r4,GPR_BUF_OFFSET(4)(r1)
	l.sw	4(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(5)(r1)
	l.sw	8(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(6)(r1)
	l.sw	12(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(7)(r1)
	l.sw	16(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(8)(r1)
	l.sw	20(r3),r4

	// Store struct pointer to callee-saved register
	l.or	r14,r3,r0
	// Store r9 to callee-saved register
	l.or	r16,r11,r0

	// Jump to gzll syscall handler
	// TODO: delay slot macro
	l.jal	gzll_syscall_handler
	l.nop

	// Store return to register
	l.lwz	r11,24(r14)
	l.sw	GPR_BUF_OFFSET(11)(r1),r11

	// todo: delay slot macro
	l.jalr	r16
	l.nop
