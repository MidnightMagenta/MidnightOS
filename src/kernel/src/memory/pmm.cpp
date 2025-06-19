#include "../../include/memory/pmm.hpp"
#include "../../include/IO/kprint.hpp"

MdOS::Result MdOS::Memory::PhysicalMemoryManager::init(MemMap *memMap) {
	if (m_initialized) { return MdOS::Result::ALREADY_INITIALIZED; }
	m_initialized = true;

	uintptr_t lowestAddr = UINTPTR_MAX;
	uintptr_t highestAddr = 0;

	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));

		if (entry->paddr < lowestAddr) { lowestAddr = entry->paddr; }
		if ((entry->paddr + entry->pageCount * 0x1000) > highestAddr) {
			highestAddr = (entry->paddr + entry->pageCount * 0x1000);
		}
	}

	if (((lowestAddr % 0x1000) != 0) || ((highestAddr % 0x1000) != 0)) {
		kprint("PMM::init: memory limits not page aligned\n");
		return MdOS::Result::INIT_FAILURE;
	}

	m_maxAvailPages = (highestAddr - lowestAddr) / 0x1000;

	if (!m_pageFrameMap.init(m_maxAvailPages, true) /*all pages marked as used*/) {
		kprint("PMM::init: failed to initialize page frame map\n");
		//TODO: panic
		return MdOS::Result::INIT_FAILURE;
	}

	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));

		if ((entry->type == EfiConventionalMemory)) {
			m_freePageCount += entry->pageCount;
			m_pageFrameMap.clear_range(entry->paddr / 0x1000, (entry->paddr / 0x1000) + entry->pageCount);
		} else if (entry->type == EfiUnusableMemory || entry->type == EfiReservedMemoryType) {
			/*void*/
		} else {
			m_reservedPageCount += entry->pageCount;
		}
	}

	m_unusablePageCount = m_maxAvailPages - (m_freePageCount + m_reservedPageCount);

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::PhysicalMemoryManager::alloc_pages(MdOS::Memory::PhysicalMemoryAllocation *alloc) {
	if (!m_initialized) {
		kprint("PMM::alloc_pages: PMM not initialized\n");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if (m_freePageCount == 0) {
		//TODO: Panic
		kprint("PMM::alloc_pages: out of memory\n");
		return MdOS::Result::OUT_OF_MEMORY;
	}
	m_freePageCount -= 1;
	m_usedPageCount += 1;
	size_t firstFreePage = m_pageFrameMap.find_first_clear_bit();
	m_pageFrameMap.set(firstFreePage);
	alloc->numPages = 1;
	alloc->base = firstFreePage * 0x1000;
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::PhysicalMemoryManager::alloc_pages(size_t numPages,
															  MdOS::Memory::PhysicalMemoryAllocation *alloc) {
	if (numPages <= 0) {
		kprint("PMM::alloc_pages: attempted to allocate 0 pages\n");
		return MdOS::Result::INVALID_PARAMETER;
	}
	if (!m_initialized) {
		kprint("PMM::alloc_pages: PMM not initialized\n");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if (m_freePageCount < numPages) {
		kprint("PMM::alloc_pages: out of memory\n");
		return MdOS::Result::OUT_OF_MEMORY;
	}

	bool allocSuccess = false;
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

			alloc->base = lastFreeIndex * 0x1000;
			alloc->numPages = numPages;
			allocSuccess = true;
			break;
		}
	}
	return allocSuccess ? MdOS::Result::SUCCESS : MdOS::Result::OUT_OF_MEMORY;
}

MdOS::Result MdOS::Memory::PhysicalMemoryManager::free_pages(const MdOS::Memory::PhysicalMemoryAllocation &alloc) {
	if (!m_initialized) {
		kprint("PMM::free_pages: PMM not initialized\n");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((alloc.base % 0x1000) != 0) {
		//TODO: Panic
		kprint("PMM::free_pages: attempted to free a page with a non page aligned address. Address: 0x%lx\n",
			   alloc.base);
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t baseIndex = alloc.base / 0x1000;
	if (baseIndex + alloc.numPages > m_pageFrameMap.size()) {
		//TODO: Panic
		kprint("PMM::free_pages: attempted to free out of range memory. Page at index %lu Page frame map size: %lu\n",
			   baseIndex, m_pageFrameMap.size());
		return MdOS::Result::INVALID_PARAMETER;
	}

	size_t freedPages = 0;
	for (size_t i = 0; i < alloc.numPages; i++) {
		if (m_pageFrameMap[baseIndex + i]) { freedPages++; }
	}
	if (freedPages == 0) {
		kprint("PMM::free_pages: all pages in range are free\n");
		return MdOS::Result::SUCCESS;
	}
	if (m_usedPageCount < freedPages) {
		kprint("PMM::free_pages: attempted to free more pages than used\n");
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_freePageCount += freedPages;
	m_usedPageCount -= freedPages;
	m_pageFrameMap.clear_range(baseIndex, baseIndex + alloc.numPages);
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::PhysicalMemoryManager::reserve_pages(uintptr_t addr, size_t numPages) {
	if (!m_initialized) {
		kprint("PMM::reserve_pages: PMM not initialized\n");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		kprint("PMM::reserve_pages: attempted to free a page with a non page aligned address. Address: 0x%lx\n", addr);
		return MdOS::Result::INVALID_PARAMETER;
	}
	if (numPages > m_freePageCount) {
		kprint("PMM::alloc_pages: out of memory\n");
		return MdOS::Result::OUT_OF_MEMORY;
	}
	size_t index = addr / 0x1000;
	if (index + numPages > m_pageFrameMap.size()) {
		kprint("PMM::reserve_pages: attempted to reserve out of range memory. Page at index %lu Page frame map size: "
			   "%lu\n",
			   index, m_pageFrameMap.size());
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_reservedPageCount += numPages;
	m_freePageCount -= numPages;
	m_pageFrameMap.set_range(index, index + numPages);
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::PhysicalMemoryManager::unreserve_pages(uintptr_t addr, size_t numPages) {
	if (!m_initialized) {
		kprint("PMM::unreserve_pages: PMM not initialized\n");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		kprint("PMM::unreserve_pages: attempted to free a page with a non page aligned address. Address: 0x%lx\n",
			   addr);
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t baseIndex = addr / 0x1000;
	if (baseIndex + numPages > m_pageFrameMap.size()) {
		kprint("PMM::unreserve_pages: attempted to reserve out of range memory. Page at index %lu Page frame map size: "
			   "%lu\n",
			   baseIndex, m_pageFrameMap.size());
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t freedPages = 0;
	for (size_t i = 0; i < numPages; i++) {
		if (m_pageFrameMap[baseIndex + i]) { freedPages++; }
	}
	if (freedPages == 0) {
		kprint("PMM::unreserve_pages: all pages in range are free\n");
		return MdOS::Result::SUCCESS;
	}
	if (m_reservedPageCount < freedPages) {
		kprint("PMM::unreserve_pages: attempted to unreserve more pages than reserved\n");
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_reservedPageCount -= freedPages;
	m_freePageCount += freedPages;
	m_pageFrameMap.clear_range(baseIndex, baseIndex + numPages);
	return MdOS::Result::SUCCESS;
}