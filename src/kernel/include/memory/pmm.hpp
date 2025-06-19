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
	inline size_t free_page_count() const { return m_freePageCount; }
	inline size_t free_mem_size() const { return m_freePageCount * 0x1000; }
	inline size_t used_page_count() const { return m_usedPageCount; }
	inline size_t used_mem_size() const { return m_usedPageCount * 0x1000; }
	inline size_t reserved_page_count() const { return m_reservedPageCount; }
	inline size_t reserved_mem_size() const { return m_reservedPageCount * 0x1000; }

private:
	bool m_initialized = false;
	size_t m_maxAvailPages = 0;
	size_t m_unusablePageCount = 0;
	size_t m_freePageCount = 0;
	size_t m_usedPageCount = 0;
	size_t m_reservedPageCount = 0;
	utils::Bitmap<uint32_t> m_pageFrameMap;
};
}// namespace MdOS::Memory

#endif