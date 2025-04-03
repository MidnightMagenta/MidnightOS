#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include "../include/IO/tty/psf1.hpp"

struct MemoryDescriptor{
    uint32_t type;
    uint32_t pad;
    uint64_t paddr;
    uint64_t vadd;
    uint64_t pageCount;
    uint64_t attributes;
};

struct MemMap{
	MemoryDescriptor *map;
	uint64_t size;
	uint64_t key;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
};

typedef struct {
	void* bufferBase;
	uint64_t bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanline;
} GOPFramebuffer;

struct BootInfo{
	MemMap *map;
    PSF1_Font *basicFont;
    GOPFramebuffer *framebuffer;
};

#endif