#ifndef MDOS_PANIC_H
#define MDOS_PANIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <k_utils/compiler_options.h>
#include <stddef.h>
#include <stdint.h>

#define PANIC(msg, errorCode)                                                                                          \
	__asm__("cli");                                                                                                    \
	PanicParams params;                                                                                                \
	params.eCode = errorCode;                                                                                          \
	__asm__ volatile("mov %%rdi, %0;"                                                                                  \
					 "mov %%rsi, %1;"                                                                                  \
					 "mov %%rbp, %2;"                                                                                  \
					 "mov %%rsp, %3"                                                                                   \
					 : "=m"(params.originalRDI), "=m"(params.originalRSI), "=m"(params.originalRBP),                   \
					   "=m"(params.originalRSP)                                                                        \
					 :                                                                                                 \
					 : "memory");                                                                                      \
	params.file = __FILE__;                                                                                            \
	params.function = __PRETTY_FUNCTION__;                                                                             \
	params.line = __LINE__;                                                                                            \
	panic_handler(msg, &params);


typedef enum { TEST_ERROR, INIT_FAIL, INVALID_STATE, MEMORY_ERROR } ErrorCode;

typedef struct {
	uint64_t rax, rbx, rcx, rdx;
	uint64_t rdi, rsi, rbp, rsp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
} MDOS_PACKED GeneralPurposeRegisters;

typedef struct {
	uint64_t cr0, cr2, cr3, cr4, cr8;
} MDOS_PACKED ControlRegisters;

typedef struct {
	uint16_t ds, es, fs, gs, ss, cs;
} MDOS_PACKED SegmentRegisters;

typedef struct {
	uint64_t originalRDI;
	uint64_t originalRSI;
	uint64_t originalRBP;
	uint64_t originalRSP;
	ErrorCode eCode;
	const char *file;
	const char *function;
	int line;
} PanicParams;

extern void get_gp_regs(GeneralPurposeRegisters *registers);
extern void get_cr_regs(ControlRegisters *registers);
extern void get_segment_regs(SegmentRegisters *registers);
void print_stack_trace();
void MDOS_NORETURN MSOD_NOOPTIMIZE panic_handler(const char *msg, PanicParams *params);

#ifdef __cplusplus
}
#endif
#endif