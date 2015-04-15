OUTPUT_ARCH(or1k)

SECTIONS
{
    .vectors : {
        *(.vectors)
    }

    .kernel ALIGN(0x2000) : {
        skernel = .;
        _skernel = .;
        *(.text*)
        *(.init)
        *(.fini)
        *(.data)
        *(.rodata*)
        *(.ctors)
        *(.dtors)
        *(.eh_frame)
        *(.jcr)
        ekernel = .;
        _ekernel = .;
    }

    .apps ALIGN(0x2000): {
                _apps_begin = .;
                . = ALIGN(0x2000);
                _apps_end = .;
        }

    .bss ALIGN(0x2000): {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
        *(COMMON)
        _end = .;
        end = .;
    }
}
