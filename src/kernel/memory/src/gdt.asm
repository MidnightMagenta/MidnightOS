bits 64
global mdos_mem_load_gdt

section .text
mdos_mem_load_gdt:
    lgdt [rdi]
    xor rax, rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi
    mov rax, 0x08
    push rax
    push rdi
    retfq
