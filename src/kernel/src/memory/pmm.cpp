#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

size_t MdOS::Memory::PhysicalMemoryManager::m_maxAvailPages = 0;
utils::Bitmap<uint32_t> MdOS::Memory::PhysicalMemoryManager::m_pageFrameMap = utils::Bitmap<uint32_t>();
bool MdOS::Memory::PhysicalMemoryManager::m_initialized = false;

void MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	if (m_initialized) { return; }
	uintptr_t lowestAddr = 0;
	uintptr_t highestAddr = 0;
	for (MemoryDescriptor *entry = memMap->map;
		 entry < (MemoryDescriptor *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 entry = (MemoryDescriptor *) (uintptr_t(entry) + (memMap->descriptorSize))) {
		if (entry->paddr < lowestAddr) { lowestAddr = entry->paddr; }
		if ((entry->paddr + entry->pageCount * 0x1000) > highestAddr) {
			highestAddr = (entry->paddr + entry->pageCount * 0x1000);
		}
	}

	m_maxAvailPages = (highestAddr - lowestAddr) / 0x1000;

	m_pageFrameMap.init(m_maxAvailPages);

	MdOS::IO::kprint("Memory attributes:\n\tLowest addr: 0x%lx\n\tHighest addr: 0x%lx\n\tPage count: %lu\n\n",
					 lowestAddr, highestAddr, m_maxAvailPages);
}