OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kernel_entry)

SECTIONS
{
    . = 0xFFFFFFFF80000000;

    .text : ALIGN(0x1000)
    {
        *(.text)
    }

    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }

    .data : ALIGN(0x1000)
    {
        *(.data)
    }
    
    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
}