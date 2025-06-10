#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

void MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	for (MemoryDescriptor *mapEntry = memMap->map;
		 mapEntry < (MemoryDescriptor *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 mapEntry = (MemoryDescriptor *) (uintptr_t(mapEntry) + (memMap->descriptorSize))) {
            MdOS::IO::kprint("Memory entry page count: %lu\n", mapEntry->pageCount);
         }
}