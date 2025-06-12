#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

size_t MdOS::Memory::PhysicalMemoryManager::m_maxAvailPages = 0;

void MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	m_maxAvailPages = 0;
	for (MemoryDescriptor *mapEntry = memMap->map;
		 mapEntry < (MemoryDescriptor *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 mapEntry = (MemoryDescriptor *) (uintptr_t(mapEntry) + (memMap->descriptorSize))) {
		m_maxAvailPages += mapEntry->pageCount;
	}
}