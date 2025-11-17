[bits 64]
global _start
global gdt
extern idt
extern main

section .init.text
_start:
    cli
    mov r15, rdi
    xor rbp, rbp
    lea rsp, [rel _stack_top]
    push rbp
    mov rbp, rsp
    call setup_gdt
    call setup_idt
    mov rdi, r15
    call main
    cli
.hang:
    hlt
    jmp .hang

; ------------------------------------------------------------
; setup_gdt
; ------------------------------------------------------------
setup_gdt:
    lea rcx, [rel gdt]
    mov [rel gdt_desc + 2], rcx
    lgdt [rel gdt_desc]
    xor rax, rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdx
    mov rax, 0x08
    push rax
    push rdx
    retfq

; ------------------------------------------------------------
; setup_idt
; ------------------------------------------------------------
setup_idt:
    lea rdx, [rel ignore_int]
    mov eax, 0x00080000
    mov ax, dx
    mov dx, 0x8E00
    mov edi, edx
    shl rdi, 32
    or rax, rdi
    shr rdx, 32

    lea rdi, [rel idt]
    mov rcx, 256
.fill_idt:
    mov [rdi], rax
    mov [rdi + 8], rdx
    add rdi, 16
    loop .fill_idt

    lea rcx, [rel idt]
    mov [rel idt_desc + 2], rcx
    lidt [rel idt_desc]
    ret

ignore_int:
    iretq

; ------------------------------------------------------------
; Data Section
; ------------------------------------------------------------
section .data
align 8
gdt_desc:
    dw 256 * 8 - 1
    dq 0

align 8
idt_desc:
    dw 256 * 16 - 1
    dq 0

align 0x1000
gdt:
    dq 0x0000000000000000
    dq 0x00af9a000000ffff
    dq 0x00af92000000ffff
    dq 0x0000000000000000
    dq 0x00aff8000000ffff
    dq 0x00aff2000000ffff
    times 250 dq 0

; ------------------------------------------------------------
; BSS / Stack Section
; ------------------------------------------------------------
section .init.bss
align 0x1000
_stack_bottom:
    resb 0x2000
_stack_top:
