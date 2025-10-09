#include "../include/cfgparse.h"
#include "../include/debug.h"
#include "../include/elf.h"
#include "../include/fs.h"
#include "../include/memory.h"
#include "../include/types.h"
#include <efi.h>
#include <efilib.h>

#ifndef HAVE_USE_MS_ABI
#error "Compiler must support using MS ABI"
#endif

#define DIRECT_MAP_BASE 0xFFFF800000000000

EFI_STATUS map_memory(uint64_t *pml4, efi_memmap_t *const memMap, elf_loadinfo_t *const loadInfo) {
	EFI_STATUS res;

	for (size_t i = 0; i < memMap->size / memMap->descSize; ++i) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) ((char *) memMap->descs + i * memMap->descSize);
		if (desc->Type == EfiUnusableMemory || desc->Type == EfiReservedMemoryType || desc->Type == EfiMemoryMappedIO ||
			desc->Type == EfiMemoryMappedIOPortSpace || desc->Type == EfiPalCode || desc->Type == EfiPersistentMemory) {
			continue;
		}


		res = mem_map_pages(pml4, desc->PhysicalStart, desc->PhysicalStart, desc->NumberOfPages);
		if (EFI_ERROR(res)) { return res; }
		res = mem_map_pages(pml4, desc->PhysicalStart + DIRECT_MAP_BASE, desc->PhysicalStart, desc->NumberOfPages);
		if (EFI_ERROR(res)) { return res; }
	}

	for (size_t i = 0; i < loadInfo->sectionCount; ++i) {
		elf_sectioninfo_t *section =
				(elf_sectioninfo_t *) ((char *) loadInfo->sections + i * sizeof(elf_sectioninfo_t));

		res = mem_map_pages(pml4, section->reqVirt, section->phys, section->pageCount);
		if (EFI_ERROR(res)) { return res; }
	}

	return EFI_SUCCESS;
}

void load_cr3(uint64_t pml4) { __asm__ volatile("mov %0, %%rax; mov %%rax, %%cr3" ::"r"(pml4)); }

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS res = EFI_SUCCESS;
	InitializeLib(imageHandle, systemTable);

	// get boot configuration
	configinfo_t bootConfig;
	ZeroMem(&bootConfig, sizeof(configinfo_t));
	res = parse_config(imageHandle, &bootConfig);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to parse boot configuration\n\r");
		goto err_free_none;
	}

	// get the boot filesystem
	EFI_HANDLE bootPart = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *bootPartFs = NULL;
	res = find_filesystem_for_guid(&bootConfig.bootPartUUID, &bootPart, &bootPartFs);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to find boot filesystem\n\r");
		goto err_free_cfg;
	}

	// open the binary to boot
	EFI_FILE *bootRoot = NULL;
	EFI_FILE *bootBinary = NULL;
	res = bootPartFs->OpenVolume(bootPartFs, &bootRoot);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to open boot volume\n\r");
		goto err_free_bootRoot;
	}

	res = bootRoot->Open(bootRoot, &bootBinary, bootConfig.bootBinPath, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to open boot binary file\n\r");
		goto err_free_bootBin;
	}

	// load the boot binary
	elf_loadinfo_t elfLoadInfo;
	ZeroMem(&elfLoadInfo, sizeof(elf_loadinfo_t));
	res = elf_load_file(bootBinary, &elfLoadInfo, ELF_LOAD_DISCONTIG);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to load boot binary\n\r");
		goto err_free_loadedBin;
	}

	void (*_start)(bootinfo_t *) = ((__attribute__((sysv_abi)) void (*)(bootinfo_t *)) elfLoadInfo.entry);

	bootBinary->Close(bootBinary);

	// map the boot binary to it's expected vaddr
	efi_memmap_t *memMap = NULL;
	gBS->AllocatePool(EfiLoaderData, sizeof(efi_memmap_t), (void**) &memMap);
	size_t memMapBufferSize = 0;
	ZeroMem(memMap, sizeof(efi_memmap_t));
	res = efi_get_memmap(memMap, &memMapBufferSize);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to get preliminary memory map with: 0x%lx\n\r", res);
		goto err_free_loadedBin;
	}

	EFI_PHYSICAL_ADDRESS pml4Addr = 0;
	res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4Addr);
	if (EFI_ERROR(res)) { goto err_free_map; }

	uint64_t *pml4 = (uint64_t *) pml4Addr;
	ZeroMem(pml4, 0x1000);
	res = map_memory(pml4, memMap, &elfLoadInfo);
	if (EFI_ERROR(res)) {
		DBG_MSG("Failed to map memory\n\r");
		goto err_free_pageTables;
	}

	// create the boot info structure
	bootinfo_t bootInfo;
	ZeroMem(&bootInfo, sizeof(bootinfo_t));

	// exit boot services
	DBG_MSG("Exiting boot services\n\r");
	res = efi_get_memmap_norealloc(memMapBufferSize, memMap);
	if (EFI_ERROR(res)) {
		DBG_MSG("Failed to get memory map\n\r");
		goto err_free_pageTables;
	}
	res = gBS->ExitBootServices(imageHandle, memMap->key);
	if (EFI_ERROR(res)) {
		// TODO: attempt recovery
		while (1) { __asm__ volatile("hlt"); }
	}

	bootInfo.memMap = memMap;

	// pass control to the loaded binary
	load_cr3((uint64_t) pml4Addr);
	_start(&bootInfo);

	while (1) { __asm__ volatile("hlt"); }// should be unreachable.

// abnormal exit path - only valid before exiting boot services
err_free_pageTables:
	mem_free_tables(pml4);
	gBS->FreePages((EFI_PHYSICAL_ADDRESS) (UINTN) pml4, 1);
err_free_map:
	efi_free_memmap(memMap);
err_free_loadedBin:
	elf_free_sections(&elfLoadInfo);
	elf_free_loadinfo(&elfLoadInfo);
err_free_bootBin:
	bootBinary->Close(bootBinary);
err_free_bootRoot:
	bootRoot->Close(bootRoot);
err_free_cfg:
	free_config(&bootConfig);
err_free_none:
	return res;
}