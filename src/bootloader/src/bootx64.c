#include "../include/basics.h"
#include "../include/efi_map.h"
#include "../include/elf_loader.h"
#include "../include/memory.h"
#include "../include/psf.h"
#include "../include/types.h"
#include "../include/video.h"
#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_STATUS get_final_EFI_map(EFI_SYSTEM_TABLE *systemTable, MemMap *map, uint64_t *pml4) {
	systemTable->BootServices->FreePool(map->map);
	size_t mapSize = get_efi_map_size(systemTable) + (200 * sizeof(EFI_MEMORY_DESCRIPTOR));
	EFI_STATUS status = systemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void **) &map->map);
	if (status != EFI_SUCCESS) { return status; }
	map->size = mapSize;
	map_pages(systemTable, pml4, (EFI_VIRTUAL_ADDRESS) map->map, (EFI_PHYSICAL_ADDRESS) map->map,
			  ALIGN_ADDR(mapSize, 0x1000, UINTN));
	status = get_EFI_map_noalloc(systemTable, map);
	if (status != EFI_SUCCESS) { return status; }
	return EFI_SUCCESS;
}

void set_CR3_PML4(uint64_t pml4Addr) { __asm__ volatile("mov %0, %%rax; mov %%rax, %%cr3" ::"r"(pml4Addr)); }

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;
	InitializeLib(imageHandle, systemTable);

	LoadedSectionInfo *sectionInfos = NULL;
	UINTN sectionInfoCount = 0;
	Elf64_Ehdr ehdr;

	status = load_kernel(imageHandle, systemTable, &ehdr, &sectionInfoCount, &sectionInfos);
	HandleError(L"Failed to load kernel", status);

	void (*_start)(BootInfo *) = ((__attribute__((sysv_abi)) void (*)(BootInfo *)) ehdr.e_entry);

	EFI_PHYSICAL_ADDRESS pml4Addr;
	uint64_t *pml4 = NULL;
	status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4Addr);
	HandleError(L"Failed to allocate PML4", status);
	pml4 = (uint64_t *) pml4Addr;
	ZeroMem((void *) pml4, 0x1000);

	GOPFramebuffer *framebuffer = NULL;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(framebuffer), (void **) &framebuffer);
	HandleError(L"Failed to allocate memory for framebuffer info struct", status);
	status = init_GOP(systemTable, framebuffer);
	HandleError(L"Failed to initialize GOP", status);

	PSF1_Font *font = NULL;
	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_Font), (void **) &font);
	status = get_PSF1_font(imageHandle, systemTable, L"FONTS\\zap-light16.psf", font);
	HandleError(L"Failed to read PSF font", status);

	MemMap *map = NULL;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(MemMap), (void **) &map);
	HandleError(L"Failed to allocate memory for memory map", status);
	status = get_EFI_map(systemTable, map);
	HandleError(L"Failed to obtain memory map", status);

	uintptr_t bootstrapHeap = 0;
	uintptr_t bootstrapHeapVaddr = 0;
	uint64_t bootstreapHeapSize = 0;
	status = alloc_bootstrap_memory(systemTable, map, sectionInfos, sectionInfoCount, &bootstreapHeapSize,
									&bootstrapHeap, &bootstrapHeapVaddr);
	HandleError(L"Failed to allocate bootstrap heap", status);

	Print(L"Created boot heap: paddr: 0x%lx vaddr: 0x%lx size: %lu\n\r", bootstrapHeap, bootstrapHeapVaddr, bootstreapHeapSize * 0x1000);

#if VERBOSE_REPORTING
	Print(L"Doing mapping pass...\n\r");
#endif
	status = get_EFI_map(systemTable, map);
	HandleError(L"Failed to obtain memory map", status);
	status = map_mem(systemTable, pml4, map, sectionInfos, sectionInfoCount);
	HandleError(L"Failed to map memory", status);

	status = map_pages(systemTable, pml4, (EFI_VIRTUAL_ADDRESS) framebuffer->bufferBase,
					   (EFI_PHYSICAL_ADDRESS) framebuffer->bufferBase, (framebuffer->bufferSize + 0x1000 - 1) / 0x1000);
	HandleError(L"Failed to map memory for the framebuffer", status);
	status = map_pages(systemTable, pml4, (EFI_VIRTUAL_ADDRESS) bootstrapHeapVaddr,
					   (EFI_PHYSICAL_ADDRESS) bootstrapHeap, bootstreapHeapSize);
	HandleError(L"Failed to map memory for the bootstrap heap", status);

	BootInfo bootInfo;
	bootInfo.map = map;
	bootInfo.pml4 = pml4;
	bootInfo.bootExtra.framebuffer = framebuffer;
	bootInfo.bootExtra.basicFont = font;
	bootInfo.bootstrapMem.baseAddr = (void *) bootstrapHeapVaddr;
	bootInfo.bootstrapMem.topAddr = (uint64_t *) ((uint64_t) bootstrapHeapVaddr + (bootstreapHeapSize * 0x1000));
	bootInfo.bootstrapMem.basePaddr = (void *) bootstrapHeap;
	bootInfo.bootstrapMem.topPaddr = (uint64_t *) ((uint64_t) bootstrapHeap + (bootstreapHeapSize * 0x1000));
	bootInfo.bootstrapMem.size = bootstreapHeapSize * 0x1000;

	get_final_EFI_map(systemTable, map, pml4);
	HandleError(L"Failed to get final EFI map", status);

	status = systemTable->BootServices->ExitBootServices(imageHandle, map->key);
	if (status != EFI_SUCCESS) {
		if (status == EFI_INVALID_PARAMETER) {
			int exitStatus = 0;
			for (int i = 0; i < 10; i++) {
				get_final_EFI_map(systemTable, map, pml4);
				status = systemTable->BootServices->ExitBootServices(imageHandle, map->key);
				if (status == EFI_SUCCESS) {
					exitStatus = 1;
					break;
				}
			}
			if (!exitStatus) { return status; }
		} else {
			return status;
		}
	}

	set_CR3_PML4((uint64_t) pml4 | PAGE_WSP);
	_start(&bootInfo);

	return EFI_SUCCESS;
}