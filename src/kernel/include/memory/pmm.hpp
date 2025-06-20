#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"
#include "../../include/efi_structs.hpp"
#include "../../include/memory/memory_types.hpp"
#include "../../include/result.hpp"

namespace MdOS::Memory::PMM {
struct PhysicalMemoryAllocation {
	PhysicalAddress base;
	size_t numPages;
};

MdOS::Result init(MemMap *memMap);

MdOS::Result alloc_pages(MdOS::Memory::PMM::PhysicalMemoryAllocation *alloc);
MdOS::Result alloc_pages(size_t numPages, MdOS::Memory::PMM::PhysicalMemoryAllocation *alloc);
MdOS::Result free_pages(const MdOS::Memory::PMM::PhysicalMemoryAllocation &alloc);
MdOS::Result reserve_pages(PhysicalAddress addr, size_t numPages);
MdOS::Result unreserve_pages(PhysicalAddress addr, size_t numPages);

MemSize max_page_count();
MemSize max_mem_size();
MemSize unusable_page_count();
MemSize unusable_mem_size();
MemSize usable_page_count();
MemSize usable_mem_size();
MemSize free_page_count();
MemSize free_mem_size();
MemSize used_page_count();
MemSize used_mem_size();
MemSize reserved_page_count();
MemSize reserved_mem_size();

MemSize min_page_index();
MemSize min_page_addr();
MemSize max_page_index();
MemSize max_page_addr();
}// namespace MdOS::Memory::PMM

#endif