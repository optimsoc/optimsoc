OUTPUT_ARCH(or32)
SEARCH_DIR(.)
__DYNAMIC  =  0;

MEMORY
{
	mem     : ORIGIN = 0x2000, LENGTH = (256K - 0x2000)
}

ENTRY(init)

PROVIDE (stack = 256K - 4);

SECTIONS
{	
	.rts : {
		*(.rtsinit)
	} > mem
	
	.init : {
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
	} > mem
	
	.text : {
		*(.text)
	}  > mem

	.data : {
		*(.data)
	} > mem

	.rodata : {
		*(.rodata);
		*(.rodata.*)
	} > mem	

	.bss : {
		*(.bss)
		*(COMMON)
		_end = ALIGN(0x8);
	} > mem
}