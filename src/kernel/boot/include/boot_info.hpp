#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "../../IO/include/psf.hpp"
#include "../include/efi_structs.hpp"
#include <stdint.h>
#include <stddef.h>

struct MemMap {
	EFI_MEMORY_DESCRIPTOR *map;
	uint64_t size;
	uint64_t key;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
};

typedef struct {
	void *bufferBase;
	uint64_t bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanline;
} GOPFramebuffer;

struct BootExtra{
	PSF1_Font *basicFont;
	GOPFramebuffer *framebuffer;
};

struct BootstrapMemoryRegion{
	void *baseAddr;
	void *topAddr;
	void *basePaddr;
	void *topPaddr;
	size_t size;
};

struct BootInfo {
	MemMap *map;
	uint64_t* pml4;
	BootExtra bootExtra;
	BootstrapMemoryRegion bootstrapMem;
};

#endif