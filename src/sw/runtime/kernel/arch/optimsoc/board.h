#ifndef _BOARD_H_
#define _BOARD_H_

#define IN_CLK  	      50000000 // Hz

#define STACKSIZE			8192
#define NUMCORES 			1

//
// Defines for each core (memory map base, OR1200 interrupt line number, etc.)
//
#define RAM_BASE            0x00000000

#define UART0_BASE  	    0x90000000
#define UART0_IRQ                    2
#define UART0_BAUD_RATE 	115200

#define SPI0_BASE 0xb0000000

//
// OR1200 tick timer period define
//
#define TICKS_PER_SEC   1000 // TODO: by increasing this the system gets in trouble, only activating debug helps then, check!

#define PHYSMEM_START 0x00040000
#define PHYSMEM_END   0x000fffff

#define PAGESIZE 0x2000
#define PAGEBITS 13
#define PAGEMASK 0x1fff

#define PAGETABLE_ENTRIES 128
#define PAGETABLE_SIZE    0x100000
#define PAGETABLE_BITS    20
#define PAGETABLE_MASK    0x0fffff

#define PAGETABLEENTRY_BITS  7
#define PAGETABLEENTRY_MASK  0x000fe000

#define PAGETABLEENTRY_VPN   0xfffff000
#define PAGETABLEENTRY_VALID 0x00000001

#define STACK_SIZE 256 // in words
#define PAGE_SIZE  0x2000
#define PAGE_BITS  13
#define ITLB_PR_MASK	(SPR_ITLBTR_SXE | SPR_ITLBTR_UXE)
#define DTLB_PR_MASK	(SPR_DTLBTR_URE | SPR_DTLBTR_UWE | SPR_DTLBTR_SRE | SPR_DTLBTR_SWE)

#endif
