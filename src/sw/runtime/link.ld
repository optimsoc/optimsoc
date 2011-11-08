OUTPUT_ARCH(or32)
SEARCH_DIR(.)
__DYNAMIC  =  0;

MEMORY
{
	vectors : ORIGIN = 0, LENGTH = 0x2000
	ram     : ORIGIN = 0x2000, LENGTH = (1M - 0x2000)
}


/*
 * Allocate the stack to be at the top of memory, since the stack
 * grows down.
 */
PROVIDE (stack = 256K - 4);
PROVIDE (_stack = 256K - 4);

SECTIONS
{	
	multicore_boot_barrier = 0x50;
	.vectors :
	{	
		*(.vectors)
	} > vectors
                              
	.text : {
		_stext = .;
		*(.text)
		_etext  =  .;
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		LONG(0)
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;
		*(.lit)
		*(.shdata)
		_endtext = .;
	}  > ram

	.rodata : {
		*(.rodata);
		*(.rodata.*)
	} > ram
	
	.shbss :
	{
		*(.shbss)
	} > ram
	
	.talias :
	{ 
	}  > ram
	
	.data : {
		sdata  =  .;
		_sdata  =  .;
		kernel/libkernel.a(.data)
		edata  =  .;
		_edata  =  .;
	} > ram
	
	.bss SIZEOF(.data) + ADDR(.data) :
	{
		sbss = . ;
		_sbss = . ;
		__bss_start = ALIGN(0x8);
		___bss_start = ALIGN(0x8);
		*(.bss)
		*(COMMON)
		ebss = .;
		_ebss = .;
		__bss_end = ALIGN(0x8);
		___bss_end = ALIGN(0x8);
		end = ALIGN(0x8);
		_end = ALIGN(0x8);
		__end = ALIGN(0x8);
	}

	.stab  0 (NOLOAD) : 
	{
		[ .stab ]
	}
	
	.stabstr  0 (NOLOAD) :
	{
		[ .stabstr ]
	}

	.app 0x40000: {
		_app_begin = .;
		apps/app0/app0bin.o(.data)
		. = ALIGN(0x2000);
		apps/app1/app1bin.o(.data)		
		_app_end = .;
	} > ram	
}