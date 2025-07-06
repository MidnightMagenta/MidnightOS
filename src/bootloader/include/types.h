#ifndef MDBOOT_BOOT_TYPES_H
#define MDBOOT_BOOT_TYPES_H

#include "../include/elf_loader.h"
#include "../include/psf.h"
#include "../include/video.h"
#include <efi.h>
#include <efilib.h>

#define BOOT_STATUS_SUCCESS 0x80808080ULL
#define BOOT_STATUS_PARTIAL 0xF0F0F0F0ULL

#define BOOT_STATUS_RUNTIME_SERVICES_REMAP_FAIL (uint64_t) (1ULL << 0)
#define BOOT_STATUS_NO_GOP (uint64_t) (1ULL << 1)
#define BOOT_STATUS_NO_FONT (uint64_t) (1ULL << 2)

typedef struct {
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN size;
	UINTN key;
	UINTN descriptorSize;
	UINT32 descriptorVersion;
	char pad[4];
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
	uint64_t bootStatus;
	uint64_t bootStatusBits;
} BootInfo;

#endif