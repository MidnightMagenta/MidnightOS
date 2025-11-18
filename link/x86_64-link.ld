OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

PHDRS {
	text PT_LOAD FLAGS(5);          /* R_E */
	data PT_LOAD FLAGS(6);          /* RW_ */
	init PT_LOAD FLAGS(7);          /* RWE */
	note PT_NOTE FLAGS(0);          /* ___ */
}

SECTIONS {
    . = 0xFFFFFFFF80000000;
    __kernel_start = .;

    .text : ALIGN(0x1000) {
        *(.text*)
        *(.rodata*)
    } :text

    .data : ALIGN(0x1000) {
        *(.data*)
    } :data

    .bss (NOLOAD) : ALIGN(0x1000) {
        *(COMMON)
        *(.bss*)
    } :data

    __kernel_end = .;
    . = ALIGN(0x1000);
    __init_start = .;

    .init.text : ALIGN(0x1000){
        *(.init.text*)
        *(.init.rodata*)
    } :init

    .init.data : ALIGN(0x1000){
        *(.init.data*)
    } :init

    .init.bss (NOLOAD) : ALIGN(0x1000){
        *(.init.bss*)
    } :init

    __init_end = .;

    /DISCARD/ : {
        *(.eh_frame)
    }
}