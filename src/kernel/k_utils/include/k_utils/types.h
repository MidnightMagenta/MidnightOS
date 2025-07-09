#ifndef MDOS_K_UTILS_TYPES_H
#define MDOS_K_UTILS_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint64_t paddr;
	uint64_t vaddr;
	uint64_t pageCount;
	uint32_t flags;
} SectionInfo;

typedef unsigned char byte_t;
typedef uintptr_t PhysicalAddress;
typedef uintptr_t VirtualAddress;

#ifdef __cplusplus
}
#endif
#endif