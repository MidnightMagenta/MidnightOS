#ifndef PMM_H
#define PMM_H

#include "../../include/bitmap.hpp"
#include "../../include/boot_info.hpp"

namespace MdOS::Memory {
class PhysicalMemoryManager {
public:
    static void init(MemMap* memMap);
private:
	static utils::Bitmap<uint32_t> m_pageFrameMap;
};
}// namespace MdOS::Memory


#endif