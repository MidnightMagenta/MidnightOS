OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80000000;
    __kernel_start = .;

    .text : ALIGN(0x1000) {
        __text_start = .;
        *(.text*)
        __text_end = .;
    }

    .rodata : ALIGN(0x1000) {
        __rodata_start = .;
        *(.rodata*)
        __rodata_end = .;
    }

    .data : ALIGN(0x1000) {
        __data_start = .;
        *(.data*)
        __data_end = .;
    }

    .bss (NOLOAD) : ALIGN(0x1000) {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
        __bss_boot_stack_start = .;
        . = ALIGN(0x1000);
        *(.bss.boot_stack)
        __bss_boot_stack_end = .;
    }

    __kernel_end = .;
}