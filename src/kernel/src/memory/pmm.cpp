#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

size_t MdOS::Memory::PhysicalMemoryManager::m_maxAvailPages = 0;
size_t MdOS::Memory::PhysicalMemoryManager::m_freePageCount = 0;
size_t MdOS::Memory::PhysicalMemoryManager::m_usedPageCount = 0;
utils::Bitmap<uint32_t> MdOS::Memory::PhysicalMemoryManager::m_pageFrameMap = utils::Bitmap<uint32_t>();
bool MdOS::Memory::PhysicalMemoryManager::m_initialized = false;

void MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	if (m_initialized) { return; }
	m_initialized = true;

	uintptr_t lowestAddr = UINTPTR_MAX;
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

	if (!m_pageFrameMap.init(m_maxAvailPages, true) /*all pages marked as used*/) {
		MdOS::IO::kprint("Failed to initialize page frame map\n");
		//TODO: panic
	}

	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map;
		 entry < (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(memMap->map) + uintptr_t(memMap->size));
		 entry = (EFI_MEMORY_DESCRIPTOR *) (uintptr_t(entry) + (memMap->descriptorSize))) {
		if ((entry->type == EfiConventionalMemory) || (entry->type == EfiBootServicesCode) ||
			(entry->type == EfiBootServicesData) || (entry->type == EfiLoaderCode)) {
			m_freePageCount += entry->pageCount;
			m_pageFrameMap.clear_range(entry->paddr / 0x1000, (entry->paddr / 0x1000) + entry->pageCount);
		}
	}
}

MdOS::Memory::PhysicalMemoryManager::Allocation MdOS::Memory::PhysicalMemoryManager::allocPage() {
	Allocation res{0, 0};
	if (m_freePageCount == 0) {
		//TODO: Panic
		MdOS::IO::kprint("Out of memory");
		return res;
	}
	m_freePageCount -= 1;
	m_usedPageCount += 1;
	size_t firstFreePage = m_pageFrameMap.find_first_clear_bit();
	m_pageFrameMap.set(firstFreePage);
	res.numPages = 1;
	res.base = firstFreePage * 0x1000;
	return res;
}

MdOS::Memory::PhysicalMemoryManager::Allocation MdOS::Memory::PhysicalMemoryManager::allocPage(size_t numPages) {
	Allocation res{0, 0};
	if (m_freePageCount < numPages) { return res; }

	size_t lastFreeIndex = m_pageFrameMap.find_first_clear_bit();
	while (lastFreeIndex < m_pageFrameMap.size()) {
		if (lastFreeIndex + numPages >= m_pageFrameMap.size()) { break; }
		bool rangeContinous = true;
		for (size_t i = 0; i < numPages; i++) {
			if (m_pageFrameMap[lastFreeIndex + i]) {
				rangeContinous = false;
				lastFreeIndex = m_pageFrameMap.find_next_clear_bit(lastFreeIndex + i + 1);
				break;
			}
		}
		if (rangeContinous) {
			for (size_t i = 0; i < numPages; i++) { m_pageFrameMap.set(lastFreeIndex + i); }
			m_freePageCount -= numPages;
			m_usedPageCount += numPages;

			res.base = lastFreeIndex * 0x1000;
			res.numPages = numPages;
			break;
		}
	}
	return res;
}

void MdOS::Memory::PhysicalMemoryManager::freePage(Allocation alloc) {
	if ((alloc.base % 0x1000) != 0) {
		//TODO: Panic
		MdOS::IO::kprint("Attempted to free a page with a non page aligned address");
		return;
	}
	if (!m_pageFrameMap[alloc.base / 0x1000]) { return; }
	m_freePageCount += alloc.numPages;
	m_usedPageCount -= alloc.numPages;
	for (size_t i = alloc.base / 0x1000; i < alloc.numPages + alloc.base / 0x1000; i++) { m_pageFrameMap.clear(i); }
}