#ifndef MDOS_PMM_H
#define MDOS_PMM_H

#include <boot/boot_info.hpp>
#include <k_utils/result.h>
#include <k_utils/types.h>
#include <memory/memory_types.hpp>

namespace MdOS::mem::phys {
struct PhysicalMemoryAllocation {
	PhysicalAddress base;
	size_t numPages;
};

struct Page {
	uint16_t flags;
	uint8_t type;
};

struct PageMetadataCreateInfo{
	uint8_t bucketSize;
	uint8_t pageType;
};

Result init(MemMap *memMap, SectionInfo *krnlSections, size_t sectionInfoCount);

Result alloc_pages(size_t numPages, uint8_t type, MdOS::mem::phys::PhysicalMemoryAllocation *alloc);
Result alloc_pages_pfm(size_t numPages, MdOS::mem::phys::PhysicalMemoryAllocation *alloc);

Result alloc_pages(size_t numPages, MdOS::mem::phys::PhysicalMemoryAllocation *alloc);
Result alloc_pages(MdOS::mem::phys::PhysicalMemoryAllocation *alloc);
uintptr_t alloc_page();

Result free_pages(const MdOS::mem::phys::PhysicalMemoryAllocation &alloc);
void free_page(uintptr_t page);

Result reserve_pages(PhysicalAddress addr, size_t numPages, uint8_t type);
Result reserve_pages(PhysicalAddress addr, size_t numPages);
Result unreserve_pages(PhysicalAddress addr, size_t numPages);

Page pfm_build_entry(const PageMetadataCreateInfo &createInfo);

Page *get_page(uintptr_t addr);

Page get_page_descriptor(uintptr_t addr);
void set_page_descriptor(uintptr_t addr, const Page &metadata);

uint8_t get_page_bucket_size(uintptr_t addr);
void set_page_bucket_size(uintptr_t addr, uint8_t order);

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