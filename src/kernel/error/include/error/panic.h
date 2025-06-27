#ifndef MDOS_PANIC_H
#define MDOS_PANIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define PANIC(msg, errorCode)                                                                                          \
	PanicParams params;                                                                                                \
	params.eCode = errorCode;                                                                                          \
	__asm__ volatile("mov %%rdi, %0;"                                                                                  \
					 "mov %%rsi, %1"                                                                                   \
					 : "=m"(params.originalRDI), "=m"(params.originalRSI)                                              \
					 :                                                                                                 \
					 : "memory");                                                                                      \
	panic_handler(msg, &params);


typedef enum { TEST_ERROR, INIT_FAIL, INVALID_STATE } ErrorCode;

typedef struct {
	uint64_t rax, rbx, rcx, rdx;
	uint64_t rdi, rsi, rbp, rsp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
} __attribute__((packed)) GeneralPurposeRegisters;

typedef struct {
	uint64_t cr0, cr2, cr3, cr4, cr8;
} __attribute__((packed)) ControlRegisters;

typedef struct {
	uint16_t ds, es, fs, gs, ss, cs;
} __attribute__((packed)) SegmentRegisters;

typedef struct {
	uint64_t originalRDI;
	uint64_t originalRSI;
	ErrorCode eCode;
} PanicParams;
extern void get_gp_regs(GeneralPurposeRegisters *registers);
extern void get_cr_regs(ControlRegisters *registers);
extern void get_segment_regs(SegmentRegisters *registers);
void print_stack_trace();
void __attribute__((noreturn)) __attribute__((optimize("O0"))) panic_handler(const char *msg, PanicParams *params);

#ifdef __cplusplus
}
#endif
#endif