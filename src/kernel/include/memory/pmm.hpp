#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"
#include "../../include/efi_structs.hpp"
#include "../../include/result.hpp"

namespace MdOS::Memory {
struct PhysicalMemoryAllocation {
	uintptr_t base;
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
	MdOS::Result reserve_pages(uintptr_t addr, size_t numPages);
	MdOS::Result unreserve_pages(uintptr_t addr, size_t numPages);

	inline size_t max_page_count() const { return m_maxAvailPages; }
	inline size_t max_mem_size() const { return m_maxAvailPages * 0x1000; }
	inline size_t unusable_page_count() const { return m_unusablePageCount; }
	inline size_t unusable_mem_size() const { return m_unusablePageCount * 0x1000; }
	inline size_t usable_page_count() const { return m_usablePageCount; }
	inline size_t usable_mem_size() const { return m_usablePageCount * 0x1000; }
	inline size_t free_page_count() const { return m_freePageCount; }
	inline size_t free_mem_size() const { return m_freePageCount * 0x1000; }
	inline size_t used_page_count() const { return m_usedPageCount; }
	inline size_t used_mem_size() const { return m_usedPageCount * 0x1000; }
	inline size_t reserved_page_count() const { return m_reservedPageCount; }
	inline size_t reserved_mem_size() const { return m_reservedPageCount * 0x1000; }

	inline size_t min_page_index() const { return m_lowestPage; }
	inline size_t min_page_addr() const { return m_lowestPage * 0x1000; }
	inline size_t max_page_index() const { return m_highestPage; }
	inline size_t max_page_addr() const { return m_highestPage * 0x1000; }

private:
	bool m_initialized = false;

	//memory trackers
	size_t m_lowestPage = 0;			//lowest addressable page reported by UEFI
	size_t m_highestPage = 0;			//highest addressable page reported by UEFI
	size_t m_maxAvailPages = 0;			//number of pages between the lowest and highest addressable pages reported by UEFI
	size_t m_unusablePageCount = 0;		//pages not backed by DRAM
	size_t m_usablePageCount = 0;		//pages backed by DRAM
	size_t m_freePageCount = 0;			//pages marked as EfiConventionalMemory or reclaimed pages backed by DRAM
	size_t m_usedPageCount = 0;			//pages allocated by the pmm
	size_t m_reservedPageCount = 0;		//pages no marked as EfiConventionalMemory, not reclaimed, but backed by DRAM
	//!memory trackers

	utils::Bitmap<uint64_t> m_pageFrameMap;
};
}// namespace MdOS::Memory

#endif