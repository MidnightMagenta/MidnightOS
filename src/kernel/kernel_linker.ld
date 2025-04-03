OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kernel_entry)

SECTIONS
{
    . = 0xFFFFFFFF80000000;

    .text : ALIGN(0x1000)
    {
        *(.init*)
        *(.text)
        *(.fini*)
    }

    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }

    .data : ALIGN(0x1000)
    {
        *(.data)
        *(.ctor*)
        *(.dtor*)
    }
    
    .eh_frame : ALIGN(0x1000) {
        *(.eh_frame)
        *(.gnu.linkonce.e*)
    }

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
}