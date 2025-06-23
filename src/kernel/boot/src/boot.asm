bits 64

; table of extern and global symbols to import/export
extern kernel_entry
extern _init
extern _fini
global _start

section .bss
	align 16
	_stack_bottom:				; kernel's bootstrap stack
		resb 0x4000
	_stack_top:

section .text
	_start:
		cli						; mask all interupts
		mov rsp, _stack_top		; set stack pointer to the kernel's stack
		push rdi				; store bootInfo prior to calling _init
		call _init
		pop rdi					; retrieve bootInfo prior to calling kernel_entry
		call kernel_entry
		call _fini
		cli
	_halt:						; halt the processor, since no shutdown routine exists yet. TODO: implement shutdown here
		hlt
		jmp _halt
