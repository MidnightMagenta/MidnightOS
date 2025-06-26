OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80000000;
    __kernel_start = .;

    .text : ALIGN(0x1000)
    {
        __text_start = .;
        *(.init*)
        *(.text)
        *(.fini*)
        __text_end = .;
    }

    .rodata : ALIGN(0x1000)
    {
        __rodata_start = .;
        *(.rodata)
        __rodata_end = .;
    }

    .data : ALIGN(0x1000)
    {
        __data_start = .;
        *(.data)
        *(.ctor*)
        *(.dtor*)
        __data_end = .;
    }

    .bss : ALIGN(0x1000)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
        . = ALIGN(0x1000);
        __bss_boot_stack_start = .;
        *(.bss.boot_stack)
        __bss_boot_stack_end = .;
    }

    __kernel_end = .;
}