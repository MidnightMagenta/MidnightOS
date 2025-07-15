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
phys::Page *m_pfm = nullptr;
phys::PhysicalMemoryMap *m_memMap = nullptr;
bool m_regionMapAvail = false;

size_t m_lastFreePage = 0;

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

static constexpr MemoryType from_efi_type(EFI_MEMORY_TYPE type) {
	switch (type) {
		case EfiReservedMemoryType:
			return MemoryType::EFI_RESERVED_MEMORY;
		case EfiLoaderCode:
			return MemoryType::EFI_RECLAIMABLE_MEMORY;
		case EfiLoaderData:
			return MemoryType::EFI_RECLAIMABLE_MEMORY;
		case EfiBootServicesCode:
			return MemoryType::EFI_RECLAIMABLE_MEMORY;
		case EfiBootServicesData:
			return MemoryType::EFI_RECLAIMABLE_MEMORY;
		case EfiRuntimeServicesCode:
			return MemoryType::EFI_RESERVED_MEMORY;
		case EfiRuntimeServicesData:
			return MemoryType::EFI_RESERVED_MEMORY;
		case EfiConventionalMemory:
			return MemoryType::FREE_MEMORY;
		case EfiUnusableMemory:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiACPIReclaimMemory:
			return MemoryType::EFI_ACPI_RECLAIMABLE_MEMORY;
		case EfiACPIMemoryNVS:
			return MemoryType::EFI_RESERVED_MEMORY;
		case EfiMemoryMappedIO:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiMemoryMappedIOPortSpace:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiPalCode:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiPersistentMemory:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiUnacceptedMemoryType:
			return MemoryType::UNUSABLE_MEMORY;
		case EfiMaxMemoryType:
			return MemoryType::UNUSABLE_MEMORY;
		default:
			break;
	}
	return MemoryType::INVALID_TYPE;
}

static size_t pfm_addr_to_index(uintptr_t addr) { return (addr >> 12) - m_lowestPage; }
static uintptr_t pfm_index_to_addr(size_t index) { return (index + m_lowestPage) << 12; }

static void pfm_set_range(uintptr_t baseAddr, size_t numPages, const phys::Page &metadata) {
	for (size_t i = pfm_addr_to_index(baseAddr); i < pfm_addr_to_index(baseAddr) + numPages; i++) {
		m_pfm[i] = metadata;
	}
}

static size_t pfm_find_first_free_page() {
	for (size_t i = 0; i < m_maxAvailPages; i++) {
		if (m_pfm[i].type == FREE_MEMORY) { return i; }
	}
	return m_maxAvailPages + 1;
}

static size_t pfm_find_next_free_page(size_t index) {
	for (size_t i = index; i < m_maxAvailPages; i++) {
		if (m_pfm[i].type == FREE_MEMORY) { return i; }
	}
	return m_maxAvailPages + 1;
}

phys::Page phys::pfm_build_entry(const PageMetadataCreateInfo &createInfo) {
	phys::Page metadata{.flags = 0, .type = 0};
	metadata.flags |= createInfo.bucketSize & 0xF;
	metadata.type = createInfo.pageType;
	return metadata;
}

static phys::Page pfm_build_entry_from_type(uint8_t type) {
	phys::PageMetadataCreateInfo createInfo{.bucketSize = 0, .pageType = type};
	return pfm_build_entry(createInfo);
}

static bool find_memory_bounds(MemMap *map, size_t &lowestPage, size_t &highestPage) {
	uintptr_t lowAddr = UINTPTR_MAX;
	uintptr_t highAddr = 0;

	for (size_t i = 0; i < map->size / map->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *dsc = (EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) map->map + (i * map->descriptorSize));
		if (dsc->paddr < lowAddr) { lowAddr = dsc->paddr; }
		if (END_PAGE_ADDR(dsc->paddr, dsc->pageCount) > highAddr) {
			highAddr = END_PAGE_ADDR(dsc->paddr, dsc->pageCount);
		}
	}

	if ((lowAddr % 0x1000) != 0 || (highAddr % 0x1000) != 0) { return false; }

	lowestPage = lowAddr >> 12;
	highestPage = highAddr >> 12;
	return true;
}

static void build_pfm(MemMap *map) {
	m_unusablePageCount = m_maxAvailPages;
	for (size_t i = 0; i < map->size / map->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *dsc = (EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) map->map + (i * map->descriptorSize));
		MemoryType type = from_efi_type(EFI_MEMORY_TYPE(dsc->type));
		kassert(type != MemoryType::INVALID_TYPE);

		switch (type) {
			case MemoryType::FREE_MEMORY:
				m_freePageCount += dsc->pageCount;
				m_unusablePageCount -= dsc->pageCount;
				break;
			case MemoryType::EFI_RESERVED_MEMORY:
				m_reservedPageCount += dsc->pageCount;
				m_unusablePageCount -= dsc->pageCount;
				break;
			case MemoryType::EFI_RECLAIMABLE_MEMORY:
				m_reservedPageCount += dsc->pageCount;
				m_unusablePageCount -= dsc->pageCount;
				break;
			case MemoryType::EFI_ACPI_RECLAIMABLE_MEMORY:
				m_reservedPageCount += dsc->pageCount;
				m_unusablePageCount -= dsc->pageCount;
				break;
			default:
				break;
		}

		pfm_set_range(dsc->paddr, dsc->pageCount, pfm_build_entry_from_type(type));
	}
}

static bool init_pfm(MemMap *map) {
	m_pfm = (phys::Page *) malloc(m_maxAvailPages * sizeof(phys::Page));
	if (m_pfm == nullptr) { return false; }
	pfm_set_range(m_lowestPage << 12, m_maxAvailPages, pfm_build_entry_from_type(MemoryType::UNUSABLE_MEMORY));
	build_pfm(map);
	return true;
}

static void build_mem_map(MemMap *map) {
	for (size_t i = 0; i < map->size / map->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *dsc = (EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) map->map + (i * map->descriptorSize));
		MemoryType type = from_efi_type(EFI_MEMORY_TYPE(dsc->type));
		kassert(type != MemoryType::INVALID_TYPE);
		m_memMap->set_range(dsc->paddr, dsc->pageCount, from_efi_type(EFI_MEMORY_TYPE(dsc->type)));
	}
}

static bool init_mem_map(MemMap *map) {
	void *buffer = malloc(sizeof(phys::PhysicalMemoryMap));
	if (buffer == nullptr) { return false; }
	m_memMap = new (buffer) phys::PhysicalMemoryMap();
	if (m_memMap == nullptr) { return false; }
	build_mem_map(map);
	m_memMap->set_range(m_memMap->get_map_base(), m_memMap->get_map_size() / 0x1000, KERNEL_RESERVED_MEMORY);
	m_regionMapAvail = true;
	return true;
}

Result phys::init(MemMap *memMap, SectionInfo *krnlSections, size_t sectionInfoCount) {
	if (m_initialized) { return MDOS_ALREADY_INITIALIZED; }
	m_initialized = true;

	if (!find_memory_bounds(memMap, m_lowestPage, m_highestPage)) {
		PANIC("Failed to find memory bounds", MDOS_PANIC_INIT_FAIL);
	}
	m_maxAvailPages = m_highestPage - m_lowestPage;

	if (!init_pfm(memMap)) { PANIC("Failed to initialize the page frame map", MDOS_PANIC_INIT_FAIL); }

	m_usablePageCount = m_maxAvailPages - m_unusablePageCount;

	kassert(m_usablePageCount + m_unusablePageCount == m_maxAvailPages);
	kassert(m_freePageCount + m_usedPageCount + m_reservedPageCount == m_usablePageCount);

	if (!init_mem_map(memMap)) { PRINT_WARNING("Region memory map not available"); }

	for (size_t i = 0; i < sectionInfoCount; i++) {
		SectionInfo *section = (SectionInfo *) (uintptr_t(krnlSections) + i * sizeof(SectionInfo));
		m_memMap->set_range(section->paddr, section->pageCount, KERNEL_RESERVED_MEMORY);
	}

	return MDOS_SUCCESS;
}

Result phys::alloc_pages_pfm(size_t numPages, uint8_t type, phys::PhysicalMemoryAllocation *alloc) {
	if (numPages <= 0) { return MDOS_INVALID_PARAMETER; }
	if (!m_initialized) { return MDOS_NOT_INITIALIZED; }
	if (m_freePageCount < numPages) { return MDOS_OUT_OF_MEMORY; }

	if (m_pfm[m_lastFreePage].type != MemoryType::FREE_MEMORY) { m_lastFreePage = pfm_find_first_free_page(); }

	bool allocSuccess = false;
	size_t lastFreeIndex = m_lastFreePage;
	while (lastFreeIndex < m_maxAvailPages) {
		if (lastFreeIndex + numPages >= m_maxAvailPages) { break; }
		bool rangeContinous = true;
		for (size_t i = 0; i < numPages; i++) {
			if (m_pfm[lastFreeIndex + i].type != MemoryType::FREE_MEMORY) {
				rangeContinous = false;
				lastFreeIndex = pfm_find_next_free_page(lastFreeIndex + i + 1);
				break;
			}
		}
		if (rangeContinous) {
			m_freePageCount -= numPages;
			m_usedPageCount += numPages;

			alloc->base = pfm_index_to_addr(lastFreeIndex);
			alloc->numPages = numPages;

			pfm_set_range(alloc->base, numPages, pfm_build_entry_from_type(type));
			if (m_memMap->initialized() && m_regionMapAvail) { m_memMap->set_range(alloc->base, numPages, type); }

			allocSuccess = true;
			break;
		}
	}

	ALLOC_LOG("Allocated %lu pages at address 0x%lx", alloc->numPages, alloc->base);
	return allocSuccess ? MDOS_SUCCESS : MDOS_OUT_OF_MEMORY;
}

Result phys::alloc_pages(size_t numPages, uint8_t type, phys::PhysicalMemoryAllocation *alloc) {
	if (!m_regionMapAvail || !m_memMap->initialized()) { return alloc_pages_pfm(numPages, type, alloc); }
	if (numPages <= 0) { return MDOS_INVALID_PARAMETER; }
	if (!m_initialized) { return MDOS_NOT_INITIALIZED; }
	if (m_freePageCount < numPages) { return MDOS_OUT_OF_MEMORY; }

	PhysicalMemoryDescriptor freeRange = m_memMap->get_first_fit_range(numPages, FREE_MEMORY);
	if (freeRange.numPages < numPages) { return MDOS_OUT_OF_MEMORY; }

	bool pfmAgree = true;
	size_t endIndex = pfm_addr_to_index(END_PAGE_ADDR(freeRange.baseAddr, freeRange.numPages));
	for (size_t i = pfm_addr_to_index(freeRange.baseAddr); i < endIndex; i++) {
		if (m_pfm[i].type != MemoryType::FREE_MEMORY) { pfmAgree = false; }
	}

	if (!pfmAgree) {
		//TODO: add memMap consolidation with PFM
		return alloc_pages_pfm(numPages, type, alloc);
	}

	pfm_set_range(freeRange.baseAddr, numPages, pfm_build_entry_from_type(type));
	if (m_memMap->initialized() && m_regionMapAvail) { m_memMap->set_range(freeRange.baseAddr, numPages, type); }

	m_freePageCount -= numPages;
	m_usedPageCount += numPages;

	alloc->base = freeRange.baseAddr;
	alloc->numPages = numPages;

	ALLOC_LOG("Allocated %lu pages at address 0x%lx", alloc->numPages, alloc->base);
	return MDOS_SUCCESS;
}

Result phys::alloc_pages_pfm(size_t numPages, phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages_pfm(numPages, KERNEL_ALLOCATED_MEMORY, alloc);
}

Result phys::alloc_pages(size_t numPages, phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(numPages, KERNEL_ALLOCATED_MEMORY, alloc);
}

Result phys::alloc_pages(phys::PhysicalMemoryAllocation *alloc) {
	return alloc_pages(1, KERNEL_ALLOCATED_MEMORY, alloc);
}

uintptr_t phys::alloc_page() {
	PhysicalMemoryAllocation allocation;
	Result res = alloc_pages(&allocation);
	if (res != MDOS_SUCCESS) { return 0; }
	return allocation.base;
}

Result phys::free_pages(const phys::PhysicalMemoryAllocation &alloc) {
	if (!m_initialized) { return MDOS_NOT_INITIALIZED; }
	if ((alloc.base % 0x1000) != 0) { return MDOS_INVALID_PARAMETER; }
	size_t baseIndex = pfm_addr_to_index(alloc.base);
	if (baseIndex + alloc.numPages > m_maxAvailPages) { return MDOS_INVALID_PARAMETER; }

	size_t freedPages = 0;
	for (size_t i = 0; i < alloc.numPages; i++) {
		if (m_pfm[baseIndex + i].type != FREE_MEMORY) { freedPages++; }
	}
	if (freedPages == 0) { return MDOS_SUCCESS; }
	if (m_usedPageCount < freedPages) { return MDOS_INVALID_PARAMETER; }

	m_freePageCount += freedPages;
	m_usedPageCount -= freedPages;
	pfm_set_range(alloc.base, alloc.numPages, pfm_build_entry_from_type(FREE_MEMORY));
	if (m_memMap->initialized() && m_regionMapAvail) { m_memMap->set_range(alloc.base, alloc.numPages, FREE_MEMORY); }

	if (baseIndex < m_lastFreePage) { m_lastFreePage = baseIndex; }

	ALLOC_LOG("Freed %lu pages at address 0x%lx", freedPages, alloc.base);
	return MDOS_SUCCESS;
}

void phys::free_page(uintptr_t page) {
	PhysicalMemoryAllocation allocation;
	allocation.base = page;
	allocation.numPages = 1;
	free_pages(allocation);
}

Result phys::reserve_pages(PhysicalAddress addr, size_t numPages, uint8_t type) {
	if (!m_initialized) { return MDOS_NOT_INITIALIZED; }
	if ((addr % 0x1000) != 0) { return MDOS_INVALID_PARAMETER; }
	if (numPages > m_freePageCount) { return MDOS_OUT_OF_MEMORY; }
	size_t index = pfm_addr_to_index(addr);
	if (index + numPages > m_maxAvailPages) { return MDOS_INVALID_PARAMETER; }

	m_reservedPageCount += numPages;
	m_freePageCount -= numPages;
	pfm_set_range(addr, numPages, pfm_build_entry_from_type(type));
	if (m_memMap->initialized() && m_regionMapAvail) { m_memMap->set_range(addr, numPages, type); }

	ALLOC_LOG("Reserved %u pages at address 0x%lx", numPages, addr);
	return MDOS_SUCCESS;
}

Result phys::reserve_pages(PhysicalAddress addr, size_t numPages) {
	return reserve_pages(addr, numPages, KERNEL_RESERVED_MEMORY);
}

Result phys::unreserve_pages(PhysicalAddress addr, size_t numPages) {
	if (!m_initialized) { return MDOS_NOT_INITIALIZED; }
	if ((addr % 0x1000) != 0) { return MDOS_INVALID_PARAMETER; }
	size_t baseIndex = pfm_addr_to_index(addr);
	if (baseIndex + numPages > m_maxAvailPages) { return MDOS_INVALID_PARAMETER; }

	size_t freedPages = 0;
	for (size_t i = 0; i < numPages; i++) {
		if (m_pfm[baseIndex + i].type != FREE_MEMORY) { freedPages++; }
	}
	if (freedPages == 0) { return MDOS_SUCCESS; }
	if (m_reservedPageCount < freedPages) { return MDOS_INVALID_PARAMETER; }

	m_reservedPageCount -= freedPages;
	m_freePageCount += freedPages;
	pfm_set_range(addr, numPages, pfm_build_entry_from_type(FREE_MEMORY));
	if (m_memMap->initialized() && m_regionMapAvail) { m_memMap->set_range(addr, numPages, FREE_MEMORY); }

	if (baseIndex < m_lastFreePage) { m_lastFreePage = baseIndex; }

	ALLOC_LOG("Unreserved %lu pages at address 0x%lx", freedPages, addr);
	return MDOS_SUCCESS;
}

phys::Page *phys::get_page(uintptr_t addr) {
	if ((addr % 0x1000) != 0) { addr = ALIGN_DOWN(addr, 0x1000, uintptr_t); }
	size_t index = pfm_addr_to_index(addr);
	if (index > m_maxAvailPages) { return nullptr; }
	return &m_pfm[index];
}

phys::Page phys::get_page_descriptor(uintptr_t addr) { return *get_page(addr); }

void phys::set_page_descriptor(uintptr_t addr, const Page &metadata) { *get_page(addr) = metadata; }

uint8_t phys::get_page_bucket_size(uintptr_t addr) { return get_page_descriptor(addr).flags & 0xF; }

void phys::set_page_bucket_size(uintptr_t addr, uint8_t order) {
	Page *page = get_page(addr);
	page->flags &= static_cast<uint16_t>(~0xFU);
	page->flags |= static_cast<uint16_t>(order & uint8_t(0xF));
}

void phys::print_mem_map() { m_memMap->print_map(); }
void phys::print_mem_stats() {
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