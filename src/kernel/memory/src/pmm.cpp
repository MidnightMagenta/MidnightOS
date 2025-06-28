#include <IO/debug_print.h>
#include <error/panic.h>
#include <memory/pmm.hpp>

using namespace MdOS::Memory;

bool m_initialized = false;
utils::Bitmap<uint64_t> m_pageFrameMap;

//memory trackers
size_t m_lowestPage = 0;	   //lowest addressable page reported by UEFI
size_t m_highestPage = 0;	   //highest addressable page reported by UEFI
size_t m_maxAvailPages = 0;	   //number of pages between the lowest and highest addressable pages reported by UEFI
size_t m_unusablePageCount = 0;//pages not backed by DRAM
size_t m_usablePageCount = 0;  //pages backed by DRAM
size_t m_freePageCount = 0;	   //pages marked as EfiConventionalMemory or reclaimed pages backed by DRAM
size_t m_usedPageCount = 0;	   //pages allocated by the pmm
size_t m_reservedPageCount = 0;//pages not marked as EfiConventionalMemory, not reclaimed, but backed by DRAM
//!memory trackers

MdOS::Result PMM::init(MemMap *memMap) {
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
		PANIC("memory limits not page aligned", INIT_FAIL);
	}

	m_maxAvailPages = (highestAddr - lowestAddr) / 0x1000;

	if (!m_pageFrameMap.init(m_maxAvailPages, true) /*all pages marked as used*/) {
		PANIC("failed to initialize page frame map", INIT_FAIL);
	}

	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));

		if ((entry->type == EfiConventionalMemory)) {
			m_freePageCount += entry->pageCount;
			m_pageFrameMap.clear_range(entry->paddr / 0x1000, (entry->paddr / 0x1000) + entry->pageCount);
		} else if (entry->type == EfiUnusableMemory || entry->type == EfiReservedMemoryType ||
				   entry->type == EfiMemoryMappedIO || entry->type == EfiMemoryMappedIOPortSpace ||
				   entry->type == EfiPalCode || entry->type == EfiPersistentMemory) {
			/*void*/
		} else {
			m_reservedPageCount += entry->pageCount;
		}
	}
	
	m_usablePageCount = m_freePageCount + m_reservedPageCount;
	m_unusablePageCount = m_maxAvailPages - m_usablePageCount;
	m_lowestPage = lowestAddr / 0x1000;
	m_highestPage = highestAddr / 0x1000;

	kassert(m_usablePageCount + m_unusablePageCount == m_maxAvailPages);
	kassert(m_freePageCount + m_usedPageCount + m_reservedPageCount == m_usablePageCount);

	DEBUG_LOG("Lowest discovered address: 0x%lx\n", lowestAddr);
	DEBUG_LOG("Highest discovered address: 0x%lx\n", highestAddr);
	DEBUG_LOG("Maximum available memory: %lu MiB\n", (m_maxAvailPages * 0x1000) / 1024 / 1024);
	DEBUG_LOG("Usable memory: %lu MiB\n", (m_usablePageCount * 0x1000) / 1024 / 1024);
	DEBUG_LOG("Unusable memory: %lu MiB\n", (m_unusablePageCount * 0x1000) / 1024 / 1024);
	DEBUG_LOG("Free memory: %lu MiB\n", (m_freePageCount * 0x1000) / 1024 / 1024);
	DEBUG_LOG("Reserved memory: %lu MiB\n", (m_reservedPageCount * 0x1000) / 1024 / 1024);

	return MdOS::Result::SUCCESS;
}

MdOS::Result PMM::alloc_pages(PMM::PhysicalMemoryAllocation *alloc) {
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if (m_freePageCount == 0) {
		PRINT_ERROR("out of memory");
		return MdOS::Result::OUT_OF_MEMORY;
	}
	m_freePageCount -= 1;
	m_usedPageCount += 1;
	size_t firstFreePage = m_pageFrameMap.find_first_clear_bit();
	m_pageFrameMap.set(firstFreePage);
	alloc->numPages = 1;
	alloc->base = (firstFreePage + min_page_index()) * 0x1000;
	return MdOS::Result::SUCCESS;
}

MdOS::Result PMM::alloc_pages(size_t numPages, PMM::PhysicalMemoryAllocation *alloc) {
	if (numPages <= 0) {
		PRINT_ERROR("attempted to allocate 0 pages");
		return MdOS::Result::INVALID_PARAMETER;
	}
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if (m_freePageCount < numPages) {
		PRINT_ERROR("out of memory");
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

			alloc->base = (lastFreeIndex + min_page_index()) * 0x1000;
			alloc->numPages = numPages;
			allocSuccess = true;
			break;
		}
	}
	return allocSuccess ? MdOS::Result::SUCCESS : MdOS::Result::OUT_OF_MEMORY;
}

MdOS::Result PMM::free_pages(const PMM::PhysicalMemoryAllocation &alloc) {
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((alloc.base % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t baseIndex = (alloc.base / 0x1000) - min_page_index();
	if (baseIndex + alloc.numPages > m_pageFrameMap.size()) {
		PRINT_ERROR("attempted to free out of range memory");
		return MdOS::Result::INVALID_PARAMETER;
	}

	size_t freedPages = 0;
	for (size_t i = 0; i < alloc.numPages; i++) {
		if (m_pageFrameMap[baseIndex + i]) { freedPages++; }
	}
	if (freedPages == 0) {
		PRINT_ERROR("all pages in range are free");
		return MdOS::Result::SUCCESS;
	}
	if (m_usedPageCount < freedPages) {
		PRINT_ERROR("attempted to free more pages than used");
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_freePageCount += freedPages;
	m_usedPageCount -= freedPages;
	m_pageFrameMap.clear_range(baseIndex, baseIndex + alloc.numPages);
	return MdOS::Result::SUCCESS;
}

MdOS::Result PMM::reserve_pages(PhysicalAddress addr, size_t numPages) {
	if (!m_initialized) {
		PRINT_ERROR("PMM::reserve_pages: PMM not initialized");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MdOS::Result::INVALID_PARAMETER;
	}
	if (numPages > m_freePageCount) {
		PRINT_ERROR("out of memory");
		return MdOS::Result::OUT_OF_MEMORY;
	}
	size_t index = (addr / 0x1000) - min_page_index();
	if (index + numPages > m_pageFrameMap.size()) {
		PRINT_ERROR("attempted to reserve out of range memory");
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_reservedPageCount += numPages;
	m_freePageCount -= numPages;
	m_pageFrameMap.set_range(index, index + numPages);
	return MdOS::Result::SUCCESS;
}

MdOS::Result PMM::unreserve_pages(PhysicalAddress addr, size_t numPages) {
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MdOS::Result::NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t baseIndex = (addr / 0x1000) - min_page_index();
	if (baseIndex + numPages > m_pageFrameMap.size()) {
		PRINT_ERROR("attempted to reserve out of range memory");
		return MdOS::Result::INVALID_PARAMETER;
	}
	size_t freedPages = 0;
	for (size_t i = 0; i < numPages; i++) {
		if (m_pageFrameMap[baseIndex + i]) { freedPages++; }
	}
	if (freedPages == 0) {
		PRINT_ERROR("all pages in range are free");
		return MdOS::Result::SUCCESS;
	}
	if (m_reservedPageCount < freedPages) {
		PRINT_ERROR("attempted to unreserve more pages than reserved");
		return MdOS::Result::INVALID_PARAMETER;
	}

	m_reservedPageCount -= freedPages;
	m_freePageCount += freedPages;
	m_pageFrameMap.clear_range(baseIndex, baseIndex + numPages);
	return MdOS::Result::SUCCESS;
}

size_t PMM::max_page_count() { return m_maxAvailPages; }
size_t PMM::max_mem_size() { return m_maxAvailPages * 0x1000; }
size_t PMM::unusable_page_count() { return m_unusablePageCount; }
size_t PMM::unusable_mem_size() { return m_unusablePageCount * 0x1000; }
size_t PMM::usable_page_count() { return m_usablePageCount; }
size_t PMM::usable_mem_size() { return m_usablePageCount * 0x1000; }
size_t PMM::free_page_count() { return m_freePageCount; }
size_t PMM::free_mem_size() { return m_freePageCount * 0x1000; }
size_t PMM::used_page_count() { return m_usedPageCount; }
size_t PMM::used_mem_size() { return m_usedPageCount * 0x1000; }
size_t PMM::reserved_page_count() { return m_reservedPageCount; }
size_t PMM::reserved_mem_size() { return m_reservedPageCount * 0x1000; }

size_t PMM::min_page_index() { return m_lowestPage; }
size_t PMM::min_page_addr() { return m_lowestPage * 0x1000; }
size_t PMM::max_page_index() { return m_highestPage; }
size_t PMM::max_page_addr() { return m_highestPage * 0x1000; }