#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "../include/IO/tty/psf.hpp"
#include <stdint.h>
#include <stddef.h>

struct MemoryDescriptor {
	uint32_t type;
	uint32_t pad;
	uint64_t paddr;
	uint64_t vadd;
	uint64_t pageCount;
	uint64_t attributes;
};

struct MemMap {
	MemoryDescriptor *map;
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