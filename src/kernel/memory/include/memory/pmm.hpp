#ifndef PMM_H
#define PMM_H

#include <boot/boot_info.hpp>
#include <boot/efi_structs.hpp>
#include <k_utils/bitmap.hpp>
#include <k_utils/result.hpp>
#include <memory/memory_types.hpp>

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