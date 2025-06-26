#ifndef MDOS_MEMORY_LIMITS_H
#define MDOS_MEMORY_LIMITS_H

extern "C" char __kernel_start;
extern "C" char __kernel_end;
extern "C" char __text_start;
extern "C" char __text_end;
extern "C" char __rodata_start;
extern "C" char __rodata_end;
extern "C" char __data_start;
extern "C" char __data_end;
extern "C" char __bss_start;
extern "C" char __bss_end;
extern "C" char __bss_boot_stack_start;
extern "C" char __bss_boot_stack_end;

#endif