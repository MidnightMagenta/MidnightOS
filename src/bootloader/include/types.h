#ifndef MDBOOT_BOOT_TYPES_H
#define MDBOOT_BOOT_TYPES_H

#include "../include/elf_loader.h"
#include "../include/psf.h"
#include "../include/video.h"
#include <efi.h>
#include <efilib.h>

typedef struct {
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN size;
	UINTN key;
	UINTN descriptorSize;
	UINT32 descriptorVersion;
} MemMap;

typedef struct {
	PSF1_Font *basicFont;
	GOPFramebuffer *framebuffer;
} BootExtra;

typedef struct {
	void *baseAddr;
	void *topAddr;
	void *basePaddr;
	void *topPaddr;
	size_t size;
} BootstrapMemoryRegion;

typedef struct {
	MemMap *map;
	uint64_t *pml4;
	BootExtra bootExtra;
	BootstrapMemoryRegion bootstrapMem;
	LoadedSectionInfo *kernelSections;
	uint64_t kernelSectionCount;
} BootInfo;

#endif