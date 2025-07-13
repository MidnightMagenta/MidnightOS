#include <IO/debug_print.h>
#include <boot/efi_structs.hpp>
#include <error/panic.h>
#include <k_utils/bitmap.hpp>
#include <k_utils/utils.hpp>
#include <memory/new.hpp>
#include <memory/physical_mem_map.hpp>
#include <memory/pmm.hpp>

using namespace MdOS::mem;

bool m_initialized = false;
MdOS::mem::phys::PageMetadata *m_pageFrameMap = nullptr;
MdOS::mem::phys::PhysicalMemoryMap *m_physicalMemoryMap = nullptr;

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

static void set_page_frame_map_range(uintptr_t baseAddr, size_t numPages,
									 const MdOS::mem::phys::PageMetadata &metadata) {
	for (size_t i = baseAddr / 0x1000; i < (baseAddr / 0x1000) + numPages; i++) { m_pageFrameMap[i] = metadata; }
}

static size_t find_first_free_memory_page() {
	for (size_t i = 0; i < m_maxAvailPages; i++) {
		if (m_pageFrameMap[i].type == FREE_MEMORY) { return i; }
	}
	return m_maxAvailPages + 1;
}

static size_t find_next_free_memory_page(size_t index) {
	for (size_t i = index; i < m_maxAvailPages; i++) {
		if (m_pageFrameMap[i].type == FREE_MEMORY) { return i; }
	}
	return m_maxAvailPages + 1;
}

Result phys::init(MemMap *memMap, SectionInfo *krnlSections, size_t sectionInfoCount) {
	if (m_initialized) { return MDOS_ALREADY_INITIALIZED; }
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

	m_pageFrameMap = (PageMetadata *) malloc(m_maxAvailPages * sizeof(PageMetadata));
	if (m_pageFrameMap == nullptr) { PANIC("failed to initialize page frame map", INIT_FAIL); }

	// pre-set page frame map entries to bucket size of order 0, no flags, type of UNUSABLE_MEMORY
	set_page_frame_map_range(0, m_maxAvailPages, PageMetadata({0, 0, MemoryType::UNUSABLE_MEMORY}));

	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));

		if (entry->type == EfiConventionalMemory) {
			// mark as free memory in the page frame map, increment m_freePageCount
			m_freePageCount += entry->pageCount;
			set_page_frame_map_range(entry->paddr, entry->pageCount, PageMetadata({0, 0, MemoryType::FREE_MEMORY}));
		} else if (entry->type == EfiUnusableMemory || entry->type == EfiReservedMemoryType ||
				   entry->type == EfiMemoryMappedIO || entry->type == EfiMemoryMappedIOPortSpace ||
				   entry->type == EfiPalCode || entry->type == EfiPersistentMemory) {
			// mark as unusable memory in the page frame map
			set_page_frame_map_range(entry->paddr, entry->pageCount, PageMetadata({0, 0, MemoryType::UNUSABLE_MEMORY}));
		} else if (entry->type == EfiACPIReclaimMemory) {
			// mark as ACPI reclaimable memory, increment m_reservedPageCount
			m_reservedPageCount += entry->pageCount;
			set_page_frame_map_range(entry->paddr, entry->pageCount,
									 PageMetadata({0, 0, MemoryType::EFI_ACPI_RECLAIMABLE_MEMORY}));
		} else if (entry->type == EfiBootServicesCode || entry->type == EfiBootServicesData ||
				   entry->type == EfiLoaderCode || entry->type == EfiLoaderData) {
			// mark as reclaimable memory, increment m_reservedPageCount
			m_reservedPageCount += entry->pageCount;
			set_page_frame_map_range(entry->paddr, entry->pageCount,
									 PageMetadata({0, 0, MemoryType::EFI_RECLAIMABLE_MEMORY}));
		} else {
			// mark as reserved memory, increment m_reservedPageCount
			m_reservedPageCount += entry->pageCount;
			set_page_frame_map_range(entry->paddr, entry->pageCount,
									 PageMetadata({0, 0, MemoryType::EFI_RESERVED_MEMORY}));
		}
	}

	m_usablePageCount = m_freePageCount + m_reservedPageCount;
	m_unusablePageCount = m_maxAvailPages - m_usablePageCount;
	m_lowestPage = lowestAddr / 0x1000;
	m_highestPage = highestAddr / 0x1000;

	void *memMapBuffer = malloc(sizeof(MdOS::mem::phys::PhysicalMemoryMap));
	m_physicalMemoryMap = new (memMapBuffer) MdOS::mem::phys::PhysicalMemoryMap();

	// build the memory range map
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

	for (size_t i = 0; i < sectionInfoCount; i++) {
		SectionInfo *section = (SectionInfo *) (uintptr_t(krnlSections) + i * sizeof(SectionInfo));
		m_physicalMemoryMap->set_range(section->paddr, section->pageCount, KERNEL_RESERVED_MEMORY);
	}

	kassert(m_usablePageCount + m_unusablePageCount == m_maxAvailPages);
	kassert(m_freePageCount + m_usedPageCount + m_reservedPageCount == m_usablePageCount);

	return MDOS_SUCCESS;
}

Result MdOS::mem::phys::alloc_pages(size_t numPages, uint8_t type, MdOS::mem::phys::PhysicalMemoryAllocation *alloc) {
	kassert(m_physicalMemoryMap->initialized());
	if (numPages <= 0) {
		PRINT_ERROR("attempted to allocate 0 pages");
		return MDOS_INVALID_PARAMETER;
	}
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MDOS_NOT_INITIALIZED;
	}
	if (m_freePageCount < numPages) {
		PRINT_ERROR("out of memory");
		return MDOS_OUT_OF_MEMORY;
	}

	PhysicalMemoryDescriptor freeRange = m_physicalMemoryMap->get_first_fit_range(numPages, FREE_MEMORY);
	if (freeRange.numPages < numPages) {
		PRINT_ERROR("Could not find sufficiently large memory range");
		return MDOS_OUT_OF_MEMORY;
	}

	size_t index = (freeRange.baseAddr / 0x1000) - min_page_index();
	set_page_frame_map_range(index, index + numPages, MdOS::mem::phys::PageMetadata({0, 0, type}));
	m_physicalMemoryMap->set_range(freeRange.baseAddr, numPages, type);

	alloc->base = freeRange.baseAddr;
	alloc->numPages = numPages;

	m_freePageCount -= numPages;
	m_usedPageCount += numPages;

	return MDOS_SUCCESS;
}

Result MdOS::mem::phys::alloc_pages_bmp(size_t numPages, MdOS::mem::phys::PhysicalMemoryAllocation *alloc) {
	if (numPages <= 0) {
		PRINT_ERROR("attempted to allocate 0 pages");
		return MDOS_INVALID_PARAMETER;
	}
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MDOS_NOT_INITIALIZED;
	}
	if (m_freePageCount < numPages) {
		PRINT_ERROR("out of memory");
		return MDOS_OUT_OF_MEMORY;
	}

	bool allocSuccess = false;
	size_t lastFreeIndex = find_first_free_memory_page();
	while (lastFreeIndex < m_maxAvailPages) {
		if (lastFreeIndex + numPages >= m_maxAvailPages) { break; }
		bool rangeContinous = true;
		for (size_t i = 0; i < numPages; i++) {
			if (m_pageFrameMap[lastFreeIndex + i].type != FREE_MEMORY) {
				rangeContinous = false;
				lastFreeIndex = find_next_free_memory_page(lastFreeIndex + i + 1);
				break;
			}
		}
		if (rangeContinous) {
			m_freePageCount -= numPages;
			m_usedPageCount += numPages;

			alloc->base = (lastFreeIndex + min_page_index()) * 0x1000;
			alloc->numPages = numPages;

			set_page_frame_map_range(lastFreeIndex, lastFreeIndex + numPages,
									 MdOS::mem::phys::PageMetadata({0, 0, KERNEL_ALLOCATED_MEMORY}));
			m_physicalMemoryMap->set_range(alloc->base, alloc->numPages, KERNEL_ALLOCATED_MEMORY);

			allocSuccess = true;
			break;
		}
	}

	return allocSuccess ? MDOS_SUCCESS : MDOS_OUT_OF_MEMORY;
}

Result phys::alloc_pages(phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(1, KERNEL_ALLOCATED_MEMORY, alloc);
}

Result phys::alloc_pages(size_t numPages, phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(numPages, KERNEL_ALLOCATED_MEMORY, alloc);
}

uintptr_t MdOS::mem::phys::alloc_page() {
	PhysicalMemoryAllocation allocation;
	Result res = alloc_pages(&allocation);
	if (res != MDOS_SUCCESS) { return 0; }
	return allocation.base;
}

Result phys::free_pages(const phys::PhysicalMemoryAllocation &alloc) {
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MDOS_NOT_INITIALIZED;
	}
	if ((alloc.base % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MDOS_INVALID_PARAMETER;
	}
	size_t baseIndex = (alloc.base / 0x1000) - min_page_index();
	if (baseIndex + alloc.numPages > m_maxAvailPages) {
		PRINT_ERROR("attempted to free out of range memory");
		return MDOS_INVALID_PARAMETER;
	}

	size_t freedPages = 0;
	for (size_t i = 0; i < alloc.numPages; i++) {
		if (m_pageFrameMap[baseIndex + i].type != FREE_MEMORY) { freedPages++; }
	}
	if (freedPages == 0) {
		PRINT_ERROR("all pages in range are free");
		return MDOS_SUCCESS;
	}
	if (m_usedPageCount < freedPages) {
		PRINT_ERROR("attempted to free more pages than used");
		return MDOS_INVALID_PARAMETER;
	}

	m_freePageCount += freedPages;
	m_usedPageCount -= freedPages;
	set_page_frame_map_range(baseIndex, baseIndex + alloc.numPages, MdOS::mem::phys::PageMetadata({0, 0, FREE_MEMORY}));
	m_physicalMemoryMap->set_range(alloc.base, alloc.numPages, FREE_MEMORY);
	return MDOS_SUCCESS;
}

void MdOS::mem::phys::free_page(uintptr_t page) {
	PhysicalMemoryAllocation allocation;
	allocation.base = page;
	allocation.numPages = 1;
	free_pages(allocation);
}

Result MdOS::mem::phys::reserve_pages(PhysicalAddress addr, size_t numPages, uint8_t type) {
	if (!m_initialized) {
		PRINT_ERROR("phys::reserve_pages: PMM not initialized");
		return MDOS_NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MDOS_INVALID_PARAMETER;
	}
	if (numPages > m_freePageCount) {
		PRINT_ERROR("out of memory");
		return MDOS_OUT_OF_MEMORY;
	}
	size_t index = (addr / 0x1000) - min_page_index();
	if (index + numPages > m_maxAvailPages) {
		PRINT_ERROR("attempted to reserve out of range memory");
		return MDOS_INVALID_PARAMETER;
	}

	m_reservedPageCount += numPages;
	m_freePageCount -= numPages;
	set_page_frame_map_range(index, index + numPages, MdOS::mem::phys::PageMetadata({0, 0, type}));
	m_physicalMemoryMap->set_range(addr, numPages, type);
	return MDOS_SUCCESS;
}

Result phys::reserve_pages(PhysicalAddress addr, size_t numPages) {
	return reserve_pages(addr, numPages, KERNEL_RESERVED_MEMORY);
}

Result phys::unreserve_pages(PhysicalAddress addr, size_t numPages) {
	if (!m_initialized) {
		PRINT_ERROR("PMM not initialized");
		return MDOS_NOT_INITIALIZED;
	}
	if ((addr % 0x1000) != 0) {
		PRINT_ERROR("attempted to free a page with a non page aligned address");
		return MDOS_INVALID_PARAMETER;
	}
	size_t baseIndex = (addr / 0x1000) - min_page_index();
	if (baseIndex + numPages > m_maxAvailPages) {
		PRINT_ERROR("attempted to reserve out of range memory");
		return MDOS_INVALID_PARAMETER;
	}
	size_t freedPages = 0;
	for (size_t i = 0; i < numPages; i++) {
		if (m_pageFrameMap[baseIndex + i].type != FREE_MEMORY) { freedPages++; }
	}
	if (freedPages == 0) {
		PRINT_ERROR("all pages in range are free");
		return MDOS_SUCCESS;
	}
	if (m_reservedPageCount < freedPages) {
		PRINT_ERROR("attempted to unreserve more pages than reserved");
		return MDOS_INVALID_PARAMETER;
	}

	m_reservedPageCount -= freedPages;
	m_freePageCount += freedPages;
	set_page_frame_map_range(baseIndex, baseIndex + numPages, MdOS::mem::phys::PageMetadata({0, 0, FREE_MEMORY}));
	m_physicalMemoryMap->set_range(addr, numPages, FREE_MEMORY);
	return MDOS_SUCCESS;
}

MdOS::mem::phys::PageMetadata *MdOS::mem::phys::get_page(uintptr_t addr) {
	if ((addr % 0x1000) != 0) { addr = ALIGN_DOWN(addr, 0x1000, uintptr_t); }
	size_t index = (addr - min_page_addr()) / 0x1000;
	if (index > m_maxAvailPages) { return nullptr; }
	return &m_pageFrameMap[index];
}

MdOS::mem::phys::PageMetadata MdOS::mem::phys::get_page_descriptor(uintptr_t addr) { return *get_page(addr); }

void MdOS::mem::phys::set_page_descriptor(uintptr_t addr, const PageMetadata &metadata) { *get_page(addr) = metadata; }

uint8_t MdOS::mem::phys::get_page_bucket_size(uintptr_t addr) { return get_page_descriptor(addr).bucketSize; }

void MdOS::mem::phys::set_page_bucket_size(uintptr_t addr, uint8_t order) { get_page(addr)->bucketSize = order; }

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