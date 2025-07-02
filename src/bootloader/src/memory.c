#include "../include/memory.h"

EFI_STATUS map_page_identity(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_PHYSICAL_ADDRESS addr) {
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS newPage;
	uint64_t *pdpt = NULL;
	uint64_t *pd = NULL;
	uint64_t *pt = NULL;

#if VERBOSE_REPORTING
	Print(L"Mapping page identity...\n\r   addr: 0x%lx\n\r", addr);
#endif

	if (!(pml4[PML4_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pml4[PML4_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pdpt = (uint64_t *) (pml4[PML4_ENTRY(addr)] & ~0xFFF);

	if (!(pdpt[PDPT_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pdpt[PDPT_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pd = (uint64_t *) (pdpt[PDPT_ENTRY(addr)] & ~0xFFF);

	if (!(pd[PD_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pd[PD_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pt = (uint64_t *) (pd[PD_ENTRY(addr)] & ~0xFFF);
	pt[PT_ENTRY(addr)] = addr | PAGE_WSP;

	return EFI_SUCCESS;
}

EFI_STATUS map_pages(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr,
					 EFI_PHYSICAL_ADDRESS paddr, UINTN pageCount) {
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS newPage = 0;
	EFI_PHYSICAL_ADDRESS physicalAddress = paddr;

#if VERBOSE_REPORTING
	Print(L"Mapping address range...\n\r   paddr: 0x%lx\n\r   vaddr: 0x%lx\n\r   page count: %u\n\r", paddr, vaddr,
		  pageCount);
#endif

	for (uint64_t address = vaddr; address < vaddr + (pageCount * 0x1000); address += 0x1000) {
		uint64_t *pdpt = NULL;
		uint64_t *pd = NULL;
		uint64_t *pt = NULL;

		if (!(pml4[PML4_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pml4[PML4_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pdpt = (uint64_t *) (pml4[PML4_ENTRY(address)] & ~0xFFF);
		if (!(pdpt[PDPT_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pdpt[PDPT_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pd = (uint64_t *) (pdpt[PDPT_ENTRY(address)] & ~0xFFF);
		if (!(pd[PD_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pd[PD_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pt = (uint64_t *) (pd[PD_ENTRY(address)] & ~0xFFF);
		pt[PT_ENTRY(address)] = physicalAddress | PAGE_WSP;
		physicalAddress += 0x1000;
	}

	return EFI_SUCCESS;
}

EFI_STATUS map_mem(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, MemMap *memMap, LoadedSectionInfo *sectionInfos,
				   UINTN sectionInfoCount) {
	EFI_STATUS status;
	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map; (char *) entry < (char *) memMap->map + memMap->size;
		 entry = (EFI_MEMORY_DESCRIPTOR *) ((char *) entry + memMap->descriptorSize)) {
		status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
		status = map_pages(systemTable, pml4, entry->PhysicalStart + DIRECT_MAP_BASE, entry->PhysicalStart,
						   entry->NumberOfPages);
		if (status != EFI_SUCCESS) { return status; }
	}

	for (LoadedSectionInfo *sectionInfo = sectionInfos;
		 (char *) sectionInfo < (char *) sectionInfos + sectionInfoCount * sizeof(LoadedSectionInfo);
		 sectionInfo = (LoadedSectionInfo *) ((char *) sectionInfo + sizeof(LoadedSectionInfo))) {
		status = map_pages(systemTable, pml4, sectionInfo->vaddr, sectionInfo->paddr, sectionInfo->pageCount);
		if (status != EFI_SUCCESS) { return status; }
	}

	return EFI_SUCCESS;
}

EFI_STATUS alloc_bootstrap_memory(EFI_SYSTEM_TABLE *systemTable, MemMap *memMap, LoadedSectionInfo *sectionInfos,
								  UINTN sectionInfoCount, uint64_t *heapSize, uintptr_t *heapAddr,
								  uintptr_t *bootstrapHeapVaddr) {
	UINTN totalMem = 0;
	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map; (char *) entry < (char *) memMap->map + memMap->size;
		 entry = (EFI_MEMORY_DESCRIPTOR *) ((char *) entry + memMap->descriptorSize)) {
		totalMem += entry->NumberOfPages;
	}

	*heapSize = totalMem / BOOTSTRAP_HEAP_RATIO;
	if (*heapSize < MINIMUM_HEAP_SIZE) { *heapSize = MINIMUM_HEAP_SIZE; }

	EFI_PHYSICAL_ADDRESS bootstrapHeapAddr;
	EFI_STATUS status =
			systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, *heapSize, &bootstrapHeapAddr);
	HandleError(L"Failed to allocate bootstrap heap", status);
	*heapAddr = (uintptr_t) bootstrapHeapAddr;
	if (!*heapAddr) { HandleError(L"Bootstrap heap is nullptr", EFI_LOAD_ERROR); }
	ZeroMem((void *) *heapAddr, *heapSize * 0x1000);

	for (unsigned int i = 0; i < sectionInfoCount; i++) {
		if ((uintptr_t) (sectionInfos[i].vaddr + sectionInfos[i].pageCount * 0x1000) > *bootstrapHeapVaddr) {
			*bootstrapHeapVaddr = (uintptr_t) (sectionInfos[i].vaddr + sectionInfos[i].pageCount * 0x1000);
		}
	}
	return EFI_SUCCESS;
}