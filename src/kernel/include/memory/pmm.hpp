#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"
#include "../../include/efi_structs.hpp"
#include "../../include/memory/memory_types.hpp"
#include "../../include/result.hpp"

namespace MdOS::Memory {
struct PhysicalMemoryAllocation {
	PhysicalAddress base;
	size_t numPages;
};

class PhysicalMemoryManager {
public:
	PhysicalMemoryManager() {}
	~PhysicalMemoryManager() {}

	MdOS::Result init(MemMap *memMap);

	MdOS::Result alloc_pages(MdOS::Memory::PhysicalMemoryAllocation *alloc);
	MdOS::Result alloc_pages(size_t numPages, MdOS::Memory::PhysicalMemoryAllocation *alloc);
	MdOS::Result free_pages(const MdOS::Memory::PhysicalMemoryAllocation &alloc);
	MdOS::Result reserve_pages(PhysicalAddress addr, size_t numPages);
	MdOS::Result unreserve_pages(PhysicalAddress addr, size_t numPages);

	inline MemSize max_page_count() const { return m_maxAvailPages; }
	inline MemSize max_mem_size() const { return m_maxAvailPages * 0x1000; }
	inline MemSize unusable_page_count() const { return m_unusablePageCount; }
	inline MemSize unusable_mem_size() const { return m_unusablePageCount * 0x1000; }
	inline MemSize usable_page_count() const { return m_usablePageCount; }
	inline MemSize usable_mem_size() const { return m_usablePageCount * 0x1000; }
	inline MemSize free_page_count() const { return m_freePageCount; }
	inline MemSize free_mem_size() const { return m_freePageCount * 0x1000; }
	inline MemSize used_page_count() const { return m_usedPageCount; }
	inline MemSize used_mem_size() const { return m_usedPageCount * 0x1000; }
	inline MemSize reserved_page_count() const { return m_reservedPageCount; }
	inline MemSize reserved_mem_size() const { return m_reservedPageCount * 0x1000; }

	inline MemSize min_page_index() const { return m_lowestPage; }
	inline MemSize min_page_addr() const { return m_lowestPage * 0x1000; }
	inline MemSize max_page_index() const { return m_highestPage; }
	inline MemSize max_page_addr() const { return m_highestPage * 0x1000; }

private:
	bool m_initialized = false;

	//memory trackers
	MemSize m_lowestPage = 0;	   //lowest addressable page reported by UEFI
	MemSize m_highestPage = 0;	   //highest addressable page reported by UEFI
	MemSize m_maxAvailPages = 0;	   //number of pages between the lowest and highest addressable pages reported by UEFI
	MemSize m_unusablePageCount = 0;//pages not backed by DRAM
	MemSize m_usablePageCount = 0;  //pages backed by DRAM
	MemSize m_freePageCount = 0;	   //pages marked as EfiConventionalMemory or reclaimed pages backed by DRAM
	MemSize m_usedPageCount = 0;	   //pages allocated by the pmm
	MemSize m_reservedPageCount = 0;//pages no marked as EfiConventionalMemory, not reclaimed, but backed by DRAM
	//!memory trackers

	utils::Bitmap<uint64_t> m_pageFrameMap;
};
}// namespace MdOS::Memory

#endif