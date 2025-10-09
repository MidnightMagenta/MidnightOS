#include "../include/memory.h"
#include "../include/debug.h"

EFI_STATUS mem_map_page(uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr, EFI_PHYSICAL_ADDRESS paddr) {
	if (pml4 == NULL) { return EFI_INVALID_PARAMETER; }
	if ((vaddr % 0x1000) != 0) { return EFI_INVALID_PARAMETER; }
	if ((paddr % 0x1000) != 0) { return EFI_INVALID_PARAMETER; }

	EFI_STATUS res;
	EFI_PHYSICAL_ADDRESS newPage;
	uint64_t *pdpt = NULL;
	uint64_t *pd = NULL;
	uint64_t *pt = NULL;

	if (!(pml4[PML4_ENTRY(vaddr)] & PAGE_PRESENT)) {
		newPage = 0;
		res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (EFI_ERROR(res)) { return res; }
		pml4[PML4_ENTRY(vaddr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pdpt = (uint64_t *) (pml4[PML4_ENTRY(vaddr)] & ~0xFFF);

	if (!(pdpt[PDPT_ENTRY(vaddr)] & PAGE_PRESENT)) {
		newPage = 0;
		res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (EFI_ERROR(res)) { return res; }
		pdpt[PDPT_ENTRY(vaddr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pd = (uint64_t *) (pdpt[PDPT_ENTRY(vaddr)] & ~0xFFF);

	if (!(pd[PD_ENTRY(vaddr)] & PAGE_PRESENT)) {
		newPage = 0;
		res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (EFI_ERROR(res)) { return res; }
		pd[PD_ENTRY(vaddr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pt = (uint64_t *) (pd[PD_ENTRY(vaddr)] & ~0xFFF);
	pt[PT_ENTRY(vaddr)] = paddr | PAGE_WSP;

	return EFI_SUCCESS;
}

EFI_STATUS mem_map_pages(uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr, EFI_PHYSICAL_ADDRESS paddr, size_t pageCount) {
	if (pml4 == NULL) { return EFI_INVALID_PARAMETER; }
	if ((vaddr % 0x1000) != 0) { return EFI_INVALID_PARAMETER; }
	if ((paddr % 0x1000) != 0) { return EFI_INVALID_PARAMETER; }

	EFI_STATUS res;

	for (size_t i = 0; i < pageCount; i++) {
		res = mem_map_page(pml4, vaddr, paddr);
		if (EFI_ERROR(res)) { return res; }
		vaddr += 0x1000;
		paddr += 0x1000;
	}

	return EFI_SUCCESS;
}

void mem_free_tables(uint64_t *pml4) {
	for (UINTN i_pml4 = 0; i_pml4 < 512; ++i_pml4) {
		uint64_t e_pml4 = pml4[i_pml4];
		if (!(e_pml4 & PAGE_PRESENT)) continue;

		uint64_t *pdpt = (uint64_t *) (e_pml4 & ADDRESS_MASK);
		for (UINTN i_pdpt = 0; i_pdpt < 512; ++i_pdpt) {
			uint64_t e_pdpt = pdpt[i_pdpt];
			if (!(e_pdpt & PAGE_PRESENT)) continue;

			uint64_t *pd = (uint64_t *) (e_pdpt & ADDRESS_MASK);
			for (UINTN i_pd = 0; i_pd < 512; ++i_pd) {
				uint64_t e_pd = pd[i_pd];
				if (!(e_pd & PAGE_PRESENT)) continue;

				uint64_t *pt = (uint64_t *) (e_pd & ADDRESS_MASK);
				gBS->FreePages((EFI_PHYSICAL_ADDRESS) (UINTN) pt, 1);
			}
			gBS->FreePages((EFI_PHYSICAL_ADDRESS) (UINTN) pd, 1);
		}
		gBS->FreePages((EFI_PHYSICAL_ADDRESS) (UINTN) pdpt, 1);
	}
}