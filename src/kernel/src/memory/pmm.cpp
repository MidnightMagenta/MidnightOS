#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

size_t MdOS::Memory::PhysicalMemoryManager::m_maxAvailPages = 0;
size_t MdOS::Memory::PhysicalMemoryManager::m_freePageCount = 0;
utils::Bitmap<uint32_t> MdOS::Memory::PhysicalMemoryManager::m_pageFrameMap = utils::Bitmap<uint32_t>();
bool MdOS::Memory::PhysicalMemoryManager::m_initialized = false;

void MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	if (m_initialized) { return; }

	uintptr_t lowestAddr = 0;
	uintptr_t highestAddr = 0;

	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map;
		 entry < (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 entry = (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(entry) + (memMap->descriptorSize))) {
		if (entry->paddr < lowestAddr) { lowestAddr = entry->paddr; }
		if ((entry->paddr + entry->pageCount * 0x1000) > highestAddr) {
			highestAddr = (entry->paddr + entry->pageCount * 0x1000);
		}
	}

	m_maxAvailPages = (highestAddr - lowestAddr) / 0x1000;

	if (!m_pageFrameMap.init(m_maxAvailPages, true)) {
		MdOS::IO::kprint("Failed to initialize page frame map\n");
		//TODO: panic
	}

	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map;
		 entry < (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 entry = (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(entry) + (memMap->descriptorSize))) {
		if ((entry->type == EfiConventionalMemory) || (entry->type == EfiBootServicesCode) ||
			(entry->type == EfiBootServicesData)) {
			m_freePageCount += entry->pageCount;
			m_pageFrameMap.clear_range(entry->paddr / 0x1000, (entry->paddr / 0x1000) + entry->pageCount);
		}
	}

	MdOS::IO::kprint("Memory attributes:\n\tLowest addr: 0x%lx\n\tHighest addr: 0x%lx\n\tPage count: %lu\n\tFree "
					 "pages: %lu\n\tTotal memory: %lu MiB\n\tAvailable memory: %lu MiB\n\n",
					 lowestAddr, highestAddr, m_maxAvailPages, m_freePageCount,
					 (m_maxAvailPages * 0x1000) / 1024 / 1024, (m_freePageCount * 0x1000) / 1024 / 1024);
}