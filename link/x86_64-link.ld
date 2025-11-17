OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80000000;
    __kernel_start = .;

    .text : ALIGN(0x1000) {
        *(.text*)
    }

    .rodata : ALIGN(0x1000) {
        *(.rodata*)
    }

    .data : ALIGN(0x1000) {
        *(.data*)
    }

    .bss (NOLOAD) : ALIGN(0x1000) {
        *(COMMON)
        *(.bss*)
    }

    __kernel_end = .;
    . = ALIGN(0x1000);
    __init_start = .;

    .init.text : ALIGN(0x1000){
        *(.init.text*)
    }

    .init.rodata : ALIGN(0x1000){
        *(.init.rodata*)
    }

    .init.data : ALIGN(0x1000){
        *(.init.data*)
    }

    .init.bss (NOLOAD) : ALIGN(0x1000){
        *(.init.bss*)
    }

    __init_end = .;
}