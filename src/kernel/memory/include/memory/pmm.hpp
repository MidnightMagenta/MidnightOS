#ifndef MDOS_PMM_H
#define MDOS_PMM_H

#include <boot/boot_info.hpp>
#include <boot/efi_structs.hpp>
#include <k_utils/bitmap.hpp>
#include <k_utils/result.hpp>
#include <k_utils/types.h>

namespace MdOS::mem::phys {
struct PhysicalMemoryAllocation {
	PhysicalAddress base;
	size_t numPages;
};

MdOS::Result init(MemMap *memMap);

MdOS::Result alloc_pages(MdOS::mem::phys::PhysicalMemoryAllocation *alloc);
MdOS::Result alloc_pages(size_t numPages, MdOS::mem::phys::PhysicalMemoryAllocation *alloc);
uintptr_t alloc_page();
void free_page(uintptr_t page);
MdOS::Result free_pages(const MdOS::mem::phys::PhysicalMemoryAllocation &alloc);
MdOS::Result reserve_pages(PhysicalAddress addr, size_t numPages);
MdOS::Result unreserve_pages(PhysicalAddress addr, size_t numPages);

void print_mem_map();
void print_mem_stats();

size_t max_page_count();
size_t max_mem_size();
size_t unusable_page_count();
size_t unusable_mem_size();
size_t usable_page_count();
size_t usable_mem_size();
size_t free_page_count();
size_t free_mem_size();
size_t used_page_count();
size_t used_mem_size();
size_t reserved_page_count();
size_t reserved_mem_size();

size_t min_page_index();
size_t min_page_addr();
size_t max_page_index();
size_t max_page_addr();
}// namespace MdOS::mem::phys

#endif