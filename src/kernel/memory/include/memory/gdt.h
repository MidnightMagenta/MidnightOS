#ifndef MDOS_GDT_H
#define MDOS_GDT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint16_t size;
	uint64_t offset;
} __attribute((packed)) GDTDescriptor;

typedef struct {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t accessByte;
	uint8_t limit1_flags;
	uint8_t base2;
} __attribute__((packed)) GDTEntry;

typedef struct {
	GDTEntry null1;
	GDTEntry kernelCode;
	GDTEntry kernelData;
	GDTEntry null2;
	GDTEntry userCode;
	GDTEntry userData;
} __attribute__((packed)) __attribute__((aligned(0x1000))) GDT;

extern GDT g_defaultGDT;
GDTDescriptor g_gdtDescriptor;
extern void load_gdt(GDTDescriptor *gdtDescriptor);

#ifdef __cplusplus
}
#endif
#endif