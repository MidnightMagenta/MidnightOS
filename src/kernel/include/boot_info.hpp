#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "../include/IO/tty/psf1.hpp"
#include <stdint.h>

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

struct BootstrapMemoryRegion{
	uint64_t *baseAddr;
	uint64_t *topAddr;
	uint64_t size;
};

struct BootInfo {
	MemMap *map;
	PSF1_Font *basicFont;
	GOPFramebuffer *framebuffer;
	BootstrapMemoryRegion bootstrapMem;
};

#endif