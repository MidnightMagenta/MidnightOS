#ifndef MDOSBOOT_MEMORY_H
#define MDOSBOOT_MEMORY_H

#include <efi.h>
#include <efilib.h>

#define PT_ENTRY(addr) (addr >> 12) & 0x1FF
#define PD_ENTRY(addr) (addr >> 21) & 0x1FF
#define PDPT_ENTRY(addr) (addr >> 30) & 0x1FF
#define PML4_ENTRY(addr) (addr >> 39) & 0x1FF

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_WSP PAGE_PRESENT | PAGE_WRITABLE
#define ADDRESS_MASK     0x000FFFFFFFFFF000ULL

EFI_STATUS mem_map_page(uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr, EFI_PHYSICAL_ADDRESS paddr);
EFI_STATUS mem_map_pages(uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr, EFI_PHYSICAL_ADDRESS paddr, size_t pageCount);
void mem_free_tables(uint64_t *pml4);

#endif