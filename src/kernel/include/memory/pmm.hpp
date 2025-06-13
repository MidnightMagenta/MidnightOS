#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"
#include "../../include/memory/efi_mem_types.hpp"

namespace MdOS::Memory {
class PhysicalMemoryManager {
public:
	static void init(MemMap *memMap);

private:
	static bool m_initialized;
	static size_t m_maxAvailPages;
	static utils::Bitmap<uint32_t> m_pageFrameMap;
};
}// namespace MdOS::Memory


#endif