#include <IO/kprint.h>
#include <error/panic.h>
#include <memory/memory_limits.h>

void print_stack_trace() {
	uint64_t *rbp;
	__asm__ volatile("mov %%rbp, %0" : "=r"(rbp));
	for (int i = 0; i < 64 && rbp != NULL; i++) {
		uint64_t rip = rbp[1];
		if (rip < (uintptr_t) (&__kernel_start)) {
			kprint("\t[trace end]");
			break;
		}
		kprint("\t#%i rbp: 0x%lx rip: 0x%lx\n", i, rbp, rip);
		rip = 0;
		rbp = (uint64_t *) rbp[0];
		if (((uintptr_t) rbp & 0x7) != 0 || (uintptr_t) rbp > (uintptr_t) (&__bss_boot_stack_end) ||
			(uintptr_t) rbp < (uintptr_t) (&__bss_boot_stack_start)) {
			kprint("\t[invalid frame. rbp: 0x%lx]", rbp);
			break;
		}
		if (!rbp) { break; }
	}
}

void panic_handler(const char *msg, PanicParams *params) {
	__asm__ volatile("cli");
	GeneralPurposeRegisters gpr;
	ControlRegisters cr;
	SegmentRegisters sr;
	uint64_t rflags = 0;
	gpr.rdi = params->originalRDI;
	gpr.rsi = params->originalRSI;
	get_gp_regs(&gpr);
	get_cr_regs(&cr);
	get_segment_regs(&sr);
	__asm__ volatile("pushfq; pop %%rax; mov %%rax, %0" : "=r"(rflags) : : "memory");
	kprint("[FATAL ERROR: 0x%x] %s\n", params->eCode, msg);
	kprint("rax: 0x%lx rbx: 0x%lx rcx: 0x%lx rdx: 0x%lx\n", gpr.rax, gpr.rbx, gpr.rcx, gpr.rdx);
	kprint("rdi: 0x%lx rsi: 0x%lx rbp: 0x%lx rsp: 0x%lx\n", gpr.rdi, gpr.rsi, gpr.rbp, gpr.rsp);
	kprint("r8:  0x%lx r9:  0x%lx r10: 0x%lx r11: 0x%lx\n", gpr.r8, gpr.r9, gpr.r10, gpr.r11);
	kprint("r12: 0x%lx r13: 0x%lx r14: 0x%lx r15: 0x%lx\n", gpr.r12, gpr.r13, gpr.r14, gpr.r15);
	kprint("cr0: 0x%lx cr2: 0x%lx cr3: 0x%lx cr4: 0x%lx cr8: 0x%lx\n", cr.cr0, cr.cr2, cr.cr3, cr.cr4, cr.cr8);
	kprint("ds: 0x%x \tes: 0x%x \tfs: 0x%x \tgs: 0x%x \tss: 0x%x \tcs: 0x%x\n", sr.ds, sr.es, sr.fs, sr.gs, sr.ss,
		   sr.cs);
	kprint("rflags: 0x%lx\n\n", rflags);
	kprint("Stack trace:\n");
	print_stack_trace();

	while (1) { __asm__ volatile("hlt"); }// TODO: if possible to shut down, shut down the computer
}