#ifndef MDOS_BOOT_INFO_H
#define MDOS_BOOT_INFO_H

#include <IO/psf.hpp>
#include <boot/efi_structs.hpp>
#include <k_utils/types.h>
#include <stddef.h>
#include <stdint.h>

#define BOOT_STATUS_SUCCESS 0x80808080ULL
#define BOOT_STATUS_PARTIAL 0xF0F0F0F0ULL

#define BOOT_STATUS_RUNTIME_SERVICES_REMAP_FAIL (uint64_t) (1ULL << 0)
#define BOOT_STATUS_NO_GOP (uint64_t) (1ULL << 1)
#define BOOT_STATUS_NO_FONT (uint64_t) (1ULL << 2)

struct MemMap {
	EFI_MEMORY_DESCRIPTOR *map;
	uint64_t size;
	uint64_t key;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
	char pad[4];
};

typedef struct {
	void *bufferBase;
	uint64_t bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanline;
	char pad[4];
} GOPFramebuffer;

struct BootExtra {
	PSF1_Font *basicFont;
	GOPFramebuffer *framebuffer;
};

struct BootstrapMemoryRegion {
	void *baseAddr;
	void *topAddr;
	void *basePaddr;
	void *topPaddr;
	size_t size;
};

struct BootInfo {
	MemMap *map;
	uint64_t *pml4;
	BootExtra bootExtra;
	BootstrapMemoryRegion bootstrapMem;
	SectionInfo *kernelSections;
	uint64_t kernelSectionCount;
	uint64_t bootStatus;
	uint64_t bootStatusBits;
};

#endif