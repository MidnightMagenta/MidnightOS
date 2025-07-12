#include <IO/debug_print.h>
#include <error/panic.h>
#include <k_utils/utils.hpp>
#include <memory/new.hpp>
#include <memory/physical_mem_map.hpp>
#include <memory/pmm.hpp>

using namespace MdOS::mem;

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

MdOS::mem::phys::PhysicalMemoryMap *m_physicalMemoryMap = nullptr;

MdOS::Result phys::init(MemMap *memMap, SectionInfo *krnlSections, size_t sectionInfoCount) {
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

		if (entry->type == EfiConventionalMemory) {
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

	void *memMapBuffer = MdOS::mem::g_bumpAlloc->alloc(sizeof(MdOS::mem::phys::PhysicalMemoryMap));
	m_physicalMemoryMap = new (memMapBuffer) MdOS::mem::phys::PhysicalMemoryMap();

	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));

		if (entry->type == EfiConventionalMemory) {
			m_physicalMemoryMap->set_range(entry->paddr, entry->pageCount, FREE_MEMORY);
		} else if (entry->type == EfiUnusableMemory || entry->type == EfiReservedMemoryType ||
				   entry->type == EfiMemoryMappedIO || entry->type == EfiMemoryMappedIOPortSpace ||
				   entry->type == EfiPalCode || entry->type == EfiPersistentMemory) {
			m_physicalMemoryMap->set_range(entry->paddr, entry->pageCount, UNUSABLE_MEMORY);
		} else if (entry->type == EfiACPIReclaimMemory) {
			m_physicalMemoryMap->set_range(entry->paddr, entry->pageCount, EFI_ACPI_RECLAIMABLE_MEMORY);
		} else if (entry->type == EfiBootServicesCode || entry->type == EfiBootServicesData ||
				   entry->type == EfiLoaderCode || entry->type == EfiLoaderData) {
			m_physicalMemoryMap->set_range(entry->paddr, entry->pageCount, EFI_RECLAIMABLE_MEMORY);
		} else {
			m_physicalMemoryMap->set_range(entry->paddr, entry->pageCount, EFI_RESERVED_MEMORY);
		}
	}

	m_physicalMemoryMap->set_range(m_physicalMemoryMap->get_map_base(), m_physicalMemoryMap->get_map_size() / 0x1000,
								   KERNEL_RESERVED_MEMORY);

	map_kernel_image(krnlSections, sectionInfoCount);

	kassert(m_usablePageCount + m_unusablePageCount == m_maxAvailPages);
	kassert(m_freePageCount + m_usedPageCount + m_reservedPageCount == m_usablePageCount);

	return MdOS::Result::SUCCESS;
}

void MdOS::mem::phys::map_kernel_image(SectionInfo *sections, size_t sectionInfoCount) {
	for (size_t i = 0; i < sectionInfoCount; i++) {
		SectionInfo *section = (SectionInfo *) (uintptr_t(sections) + i * sizeof(SectionInfo));
		m_physicalMemoryMap->set_range(section->paddr, section->pageCount, KERNEL_RESERVED_MEMORY);
	}
}

MdOS::Result MdOS::mem::phys::alloc_pages(size_t numPages, uint32_t type,
										  MdOS::mem::phys::PhysicalMemoryAllocation *alloc) {
	kassert(m_physicalMemoryMap->initialized());
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

	PhysicalMemoryDescriptor freeRange = m_physicalMemoryMap->get_first_fit_range(numPages, FREE_MEMORY);
	if (freeRange.numPages < numPages) {
		PRINT_ERROR("Could not find sufficiently large memory range");
		return MdOS::Result::OUT_OF_MEMORY;
	}

	size_t index = (freeRange.baseAddr / 0x1000) - min_page_index();
	m_pageFrameMap.set_range(index, index + numPages);
	m_physicalMemoryMap->set_range(freeRange.baseAddr, numPages, type);

	alloc->base = freeRange.baseAddr;
	alloc->numPages = numPages;

	m_freePageCount -= numPages;
	m_usedPageCount += numPages;

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::mem::phys::alloc_pages_bmp(size_t numPages, MdOS::mem::phys::PhysicalMemoryAllocation *alloc) {
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

			m_pageFrameMap.set_range(lastFreeIndex, lastFreeIndex + numPages);
			m_physicalMemoryMap->set_range(alloc->base, alloc->numPages, KERNEL_ALLOCATED_MEMORY);

			allocSuccess = true;
			break;
		}
	}

	return allocSuccess ? MdOS::Result::SUCCESS : MdOS::Result::OUT_OF_MEMORY;
}

MdOS::Result phys::alloc_pages(phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(1, KERNEL_ALLOCATED_MEMORY, alloc);
}

MdOS::Result phys::alloc_pages(size_t numPages, phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(numPages, KERNEL_ALLOCATED_MEMORY, alloc);
}

uintptr_t MdOS::mem::phys::alloc_page() {
	PhysicalMemoryAllocation allocation;
	MdOS::Result res = alloc_pages(&allocation);
	if (res != MdOS::Result::SUCCESS) { return 0; }
	return allocation.base;
}

MdOS::Result phys::free_pages(const phys::PhysicalMemoryAllocation &alloc) {
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
	m_physicalMemoryMap->set_range(alloc.base, alloc.numPages, FREE_MEMORY);
	return MdOS::Result::SUCCESS;
}

void MdOS::mem::phys::free_page(uintptr_t page) {
	PhysicalMemoryAllocation allocation;
	allocation.base = page;
	allocation.numPages = 1;
	free_pages(allocation);
}

MdOS::Result MdOS::mem::phys::reserve_pages(PhysicalAddress addr, size_t numPages, uint32_t type) {
	if (!m_initialized) {
		PRINT_ERROR("phys::reserve_pages: PMM not initialized");
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
	m_physicalMemoryMap->set_range(addr, numPages, type);
	return MdOS::Result::SUCCESS;
}

MdOS::Result phys::reserve_pages(PhysicalAddress addr, size_t numPages) {
	return reserve_pages(addr, numPages, KERNEL_RESERVED_MEMORY);
}

MdOS::Result phys::unreserve_pages(PhysicalAddress addr, size_t numPages) {
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
	m_physicalMemoryMap->set_range(addr, numPages, FREE_MEMORY);
	return MdOS::Result::SUCCESS;
}

void MdOS::mem::phys::print_mem_map() { m_physicalMemoryMap->print_map(); }
void MdOS::mem::phys::print_mem_stats() {
	DEBUG_LOG("Lowest discovered address: 0x%lx\n", min_page_addr());
	DEBUG_LOG("Highest discovered address: 0x%lx\n", max_page_addr());
	DEBUG_LOG("Maximum available memory: %lu MiB\n", max_mem_size() / 1048576);
	DEBUG_LOG("Usable memory: %lu MiB\n", usable_mem_size() / 1048576);
	DEBUG_LOG("Unusable memory: %lu MiB\n", unusable_mem_size() / 1048576);
	DEBUG_LOG("Free memory: %lu MiB\n", free_mem_size() / 1048576);
	DEBUG_LOG("Reserved memory: %lu MiB\n", reserved_mem_size() / 1048576);
}

size_t phys::max_page_count() { return m_maxAvailPages; }
size_t phys::max_mem_size() { return m_maxAvailPages * 0x1000; }
size_t phys::unusable_page_count() { return m_unusablePageCount; }
size_t phys::unusable_mem_size() { return m_unusablePageCount * 0x1000; }
size_t phys::usable_page_count() { return m_usablePageCount; }
size_t phys::usable_mem_size() { return m_usablePageCount * 0x1000; }
size_t phys::free_page_count() { return m_freePageCount; }
size_t phys::free_mem_size() { return m_freePageCount * 0x1000; }
size_t phys::used_page_count() { return m_usedPageCount; }
size_t phys::used_mem_size() { return m_usedPageCount * 0x1000; }
size_t phys::reserved_page_count() { return m_reservedPageCount; }
size_t phys::reserved_mem_size() { return m_reservedPageCount * 0x1000; }

size_t phys::min_page_index() { return m_lowestPage; }
size_t phys::min_page_addr() { return m_lowestPage * 0x1000; }
size_t phys::max_page_index() { return m_highestPage; }
size_t phys::max_page_addr() { return m_highestPage * 0x1000; }