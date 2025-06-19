#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"
#include "../../include/efi_structs.hpp"

namespace MdOS::Memory {
class PhysicalMemoryManager {
public:
	struct Allocation{
		uintptr_t base;
		size_t numPages;
	};

	static void init(MemMap *memMap);

	static Allocation allocPage();
	static Allocation allocPage(size_t numPages);
	static void freePage(Allocation alloc);
private:
	static bool m_initialized;
	static size_t m_maxAvailPages;
	static size_t m_freePageCount;
	static size_t m_usedPageCount;
	static utils::Bitmap<uint32_t> m_pageFrameMap;
};
}// namespace MdOS::Memory


#endif