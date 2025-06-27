bits 64
global get_gp_regs
global get_cr_regs
global get_segment_regs

section .text

get_gp_regs:
    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 64], r8
    mov [rdi + 72], r9
    mov [rdi + 80], r10
    mov [rdi + 88], r11
    mov [rdi + 96], r12
    mov [rdi + 104], r13
    mov [rdi + 112], r14
    mov [rdi + 120], r15
    ret

get_cr_regs:
    mov rax, cr0
    mov [rdi + 0], rax
    mov rax, cr2
    mov [rdi + 8], rax
    mov rax, cr3
    mov [rdi + 16], rax
    mov rax, cr4
    mov [rdi + 24], rax
    mov rax, cr8
    mov [rdi + 32], rax
    ret

get_segment_regs:
    mov ax, ds
    mov [rdi + 0], ax
    mov ax, es
    mov [rdi + 2], ax
    mov ax, fs
    mov [rdi + 4], ax
    mov ax, gs
    mov [rdi + 6], ax
    mov ax, ss
    mov [rdi + 8], ax
    mov ax, cs
    mov [rdi + 10], ax
    ret