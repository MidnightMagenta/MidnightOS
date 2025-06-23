#ifndef EFI_STRUCTS_H
#define EFI_STRUCTS_H

#include <stdint.h>

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiUnacceptedMemoryType,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

struct EFI_MEMORY_DESCRIPTOR {
	uint32_t type;
	uint32_t pad;
	uint64_t paddr;
	uint64_t vadd;
	uint64_t pageCount;
	uint64_t attributes;
};

#endif