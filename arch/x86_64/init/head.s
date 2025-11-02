.global _start
.global _idt
.global _gdt
.extern main

.section .text
_start:
    cli
    mov %rdi, %r15
    xor %rbp, %rbp
    mov $_stack_top, %rsp
    push %rbp
    mov %rsp, %rbp
    call setup_gdt
    call setup_idt
    mov %r15, %rdi
    call main
    cli
1:  hlt
    jmp 1b

setup_gdt:
    lea _gdt(%rip), %rcx
    mov %rcx, gdt_desc+2(%rip)
    lgdt gdt_desc(%rip)
    xor %rax, %rax
    mov $0x10, %rax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    pop %rdx
    mov $0x08, %rax
    push %rax
    push %rdx
    retfq

/*
  Load the IDT with the ignore_int procedure
*/
setup_idt:
    lea ignore_int(%rip), %rdx
    mov $0x00080000, %eax
    mov %dx, %ax
    mov $0x8E00, %dx
    mov %edx, %edi
    shl $32, %rdi
    or %rdi, %rax
    shr $32, %rdx

    lea _idt(%rip), %rdi
    mov $256, %rcx
1:  mov %rax, (%rdi)
    mov %rdx, 8(%rdi)
    add $16, %rdi
    loop 1b
    lea _idt(%rip), %rcx
    mov %rcx, idt_desc+2(%rip)
    lidt idt_desc(%rip)
    ret

ignore_int:
    iretq

.section .data
.align 8
gdt_desc:
    .2byte  256 * 8 - 1
    .8byte  0

.align 8
idt_desc:
    .2byte  256 * 16 - 1
    .8byte  0

.align 0x1000
_idt:
    .skip   256 * 16

.align 0x1000
_gdt:
    .8byte  0x0000000000000000
    .8byte  0x00af9a000000ffff
    .8byte  0x00af92000000ffff
    .8byte  0x0000000000000000
    .8byte  0x00aff8000000ffff
    .8byte  0x00aff2000000ffff
    .skip   250 * 8

.section .bss
.align 0x1000
_stack_bottom:
    .skip   0x2000
_stack_top:
