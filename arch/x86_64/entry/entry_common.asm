%ifndef _NYX_ENTRY_SH
%define _NYX_ENTRY_SH

; ------------------------------------------------------------
; Push all general-purpose registers
; ------------------------------------------------------------
%macro PUSH_REGS 0
    push rsp
    push rbp
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
%endmacro

; ------------------------------------------------------------
; Pop all general-purpose registers
; ------------------------------------------------------------
%macro POP_REGS 0
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
    pop rsp
%endmacro

%define PUSH_REGS() PUSH_REGS
%define POP_REGS()  POP_REGS

; ------------------------------------------------------------
; ENTRY macro
; Defines an interrupt entry point.
; Automatically pushes all registers and calls do_<symbol>.
; If ecode = 0, pushes a dummy error code.
; ------------------------------------------------------------
%macro ENTRY 2
    global %1
    extern do_%1
%1:
%if %2 = 0
    push 0
%endif
    PUSH_REGS
    mov rdi, rsp
    mov rbx, rsp
    and rsp, -16
    sub rsp, 8
    call do_%1
    mov rsp, rbx
    POP_REGS
%if %2 = 0
    add rsp, 8
%endif
    iretq
%endmacro

; Convenience wrappers
%define ENTRY_NECODE(sym) ENTRY sym, 0
%define ENTRY_ECODE(sym)  ENTRY sym, 1

%endif
