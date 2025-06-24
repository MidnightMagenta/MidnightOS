#ifndef MDBOOT_BOOT_MEM_MAP_H
#define MDBOOT_BOOT_MEM_MAP_H

#include "../include/basics.h"
#include "../include/efi_map.h"
#include "../include/elf_loader.h"
#include <efi.h>
#include <efilib.h>

#define PT_ENTRY(addr) (addr >> 12) & 0x1FF
#define PD_ENTRY(addr) (addr >> 21) & 0x1FF
#define PDPT_ENTRY(addr) (addr >> 30) & 0x1FF
#define PML4_ENTRY(addr) (addr >> 39) & 0x1FF

#define PAGE_PRESENT 1 << 0
#define PAGE_WRITABLE 1 << 1
#define PAGE_WSP PAGE_PRESENT | PAGE_WRITABLE

#define BOOTSTRAP_HEAP_RATIO 166
#define MINIMUM_HEAP_SIZE 8192//32 MiB

EFI_STATUS map_page_identity(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_PHYSICAL_ADDRESS addr);
EFI_STATUS map_pages(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr,
					 EFI_PHYSICAL_ADDRESS paddr, UINTN pageCount);
EFI_STATUS map_mem(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, MemMap *memMap, LoadedSectionInfo *sectionInfos,
				   UINTN sectionInfoCount);
EFI_STATUS alloc_bootstrap_memory(EFI_SYSTEM_TABLE *systemTable, MemMap *memMap, LoadedSectionInfo *sectionInfos,
								  UINTN sectionInfoCount, uint64_t *heapSize, uintptr_t *heapAddr,
								  uintptr_t *bootstrapHeapVaddr);

#endif