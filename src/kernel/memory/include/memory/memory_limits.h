#ifndef MDOS_MEMORY_LIMITS_H
#define MDOS_MEMORY_LIMITS_H
#ifdef __cplusplus
extern "C" {
#endif

extern char __kernel_start;
extern char __kernel_end;
extern char __text_start;
extern char __text_end;
extern char __rodata_start;
extern char __rodata_end;
extern char __data_start;
extern char __data_end;
extern char __bss_start;
extern char __bss_end;
extern char __bss_boot_stack_start;
extern char __bss_boot_stack_end;

#ifdef __cplusplus
}
#endif
#endif
