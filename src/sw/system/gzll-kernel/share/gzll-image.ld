OUTPUT_ARCH(or1k)

SECTIONS
{
    .vectors : {
        *(.vectors)
    }

    .kernel ALIGN(0x2000) : {
        *(.gzll_image_layout);
        *(.text*)
        *(.init)
        *(.fini)
        *(.data)
        *(.rodata*)
        *(.ctors)
        *(.dtors)
        *(.eh_frame)
        *(.jcr)
        _image_gzll_kernel_end = .;
    }

    .bss ALIGN(0x2000): {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        _end = .;
        end = .;
    }

    .apps ALIGN(0x2000): {
        _image_gzll_apps_start = .;
        *(.app_data)
        _image_gzll_apps_end = .;
     }

}
