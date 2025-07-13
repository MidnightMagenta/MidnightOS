#include <IO/debug_print.h>
#include <error/panic.h>
#include <memory/phys_virt_conversion.h>
#include <memory/physical_mem_map.hpp>
#include <memory/pmm.hpp>

MdOS::mem::phys::PhysicalMemoryMap::PhysicalMemoryMap() {
	m_initialized = false;
	MdOS::mem::phys::PhysicalMemoryAllocation allocation;
	MdOS::mem::phys::alloc_pages_bmp(2, &allocation);
	m_mapAllocator.init((void *) (MDOS_PHYS_TO_VIRT(allocation.base)), 2 * 0x1000, sizeof(PhysicalMapEntry));
	m_numberOfEntries = 0;
	m_map = get_entry();
	m_map->physicalBase = 0;
	m_map->numPages = MdOS::mem::phys::max_page_count();
	m_map->type = UNUSABLE_MEMORY;
	m_map->next = nullptr;
	m_map->prev = nullptr;

	m_mapBase = allocation.base;
	m_mapSize = allocation.numPages * 0x1000;
	m_initialized = true;
}

MdOS::mem::phys::PhysicalMemoryMap::~PhysicalMemoryMap() {
	MdOS::mem::phys::PhysicalMemoryAllocation allocation;
	allocation.base = m_mapBase;
	allocation.numPages = m_mapSize / 0x1000;
	MdOS::mem::phys::free_pages(allocation);
	m_initialized = false;
}

void MdOS::mem::phys::PhysicalMemoryMap::set_range(uintptr_t addr, size_t numPages, uint8_t type) {
	if (!m_initialized) { return; }
	uintptr_t endAddr = addr + numPages * 0x1000;

	PhysicalMapEntry *entry = m_map;
	while (entry != nullptr) {
		uintptr_t entryStart = entry->physicalBase;
		uintptr_t entryEnd = entryStart + entry->numPages * 0x1000;

		if (entryEnd <= addr || entryStart >= endAddr) {
			entry = entry->next;
			continue;
		}

		PhysicalMapEntry *next = entry->next;

		if (entryStart < addr) {
			PhysicalMapEntry *left = get_entry();
			left->physicalBase = entryStart;
			left->numPages = (addr - entryStart) / 0x1000;
			left->type = entry->type;
			left->prev = entry->prev;
			left->next = nullptr;
			if (left->prev) {
				left->prev->next = left;
			} else {
				m_map = left;
			}
			entry->prev = left;
			left->next = entry;
		}

		if (entryEnd > endAddr) {
			PhysicalMapEntry *right = get_entry();
			right->physicalBase = endAddr;
			right->numPages = (entryEnd - endAddr) / 0x1000;
			right->type = entry->type;
			right->next = entry->next;
			right->prev = nullptr;
			if (right->next) { right->next->prev = right; }
			entry->next = right;
			right->prev = entry;
		}

		if (entry->prev) { entry->prev->next = entry->next; }
		if (entry->next) { entry->next->prev = entry->prev; }
		if (entry == m_map) { m_map = entry->next; }
		free_entry(entry);

		entry = next;
	}

	PhysicalMapEntry *newEntry = get_entry();
	newEntry->physicalBase = addr;
	newEntry->numPages = numPages;
	newEntry->type = type;

	if (m_map == nullptr || addr < m_map->physicalBase) {
		newEntry->next = m_map;
		newEntry->prev = nullptr;
		if (m_map) { m_map->prev = newEntry; }
		m_map = newEntry;
	} else {
		PhysicalMapEntry *cur = m_map;
		while (cur->next && cur->next->physicalBase < addr) { cur = cur->next; }
		newEntry->next = cur->next;
		newEntry->prev = cur;
		if (cur->next) { cur->next->prev = newEntry; }
		cur->next = newEntry;
	}

	clean_map();
}

void MdOS::mem::phys::PhysicalMemoryMap::print_map() {
	PhysicalMapEntry *entry = m_map;
	while (entry != nullptr) {
		DEBUG_LOG("Base: 0x%lx | Size: %lu KiB | Type: %u\n", entry->physicalBase, (entry->numPages * 0x1000) / 1024,
				  entry->type);
		entry = entry->next;
	}
}

MdOS::mem::phys::PhysicalMemoryDescriptor MdOS::mem::phys::PhysicalMemoryMap::get_first_range(uint8_t type) {
	PhysicalMapEntry *entry = m_map;
	PhysicalMemoryDescriptor res{0, 0, INVALID_TYPE};

	while (entry != nullptr) {
		if (entry->type == type) {
			res.baseAddr = entry->physicalBase;
			res.numPages = entry->numPages;
			res.type = entry->type;
			return res;
		}
		entry = entry->next;
	}
	return res;
}

MdOS::mem::phys::PhysicalMemoryDescriptor MdOS::mem::phys::PhysicalMemoryMap::get_next_range(uintptr_t addr,
																							 uint8_t type) {
	PhysicalMapEntry *entry = m_map;
	PhysicalMemoryDescriptor res{0, 0, INVALID_TYPE};

	while (entry != nullptr) {
		if (entry->type == type && entry->physicalBase >= addr) {
			res.baseAddr = entry->physicalBase;
			res.numPages = entry->numPages;
			res.type = entry->type;
			return res;
		}
		entry = entry->next;
	}
	return res;
}

MdOS::mem::phys::PhysicalMemoryDescriptor MdOS::mem::phys::PhysicalMemoryMap::get_first_fit_range(size_t numPages,
																								  uint8_t type) {
	PhysicalMapEntry *entry = m_map;
	PhysicalMemoryDescriptor res{0, 0, INVALID_TYPE};

	while (entry != nullptr) {
		if (entry->type == type && entry->numPages >= numPages) {
			res.baseAddr = entry->physicalBase;
			res.numPages = entry->numPages;
			res.type = entry->type;
			return res;
		}
		entry = entry->next;
	}
	return res;
}

uint8_t MdOS::mem::phys::PhysicalMemoryMap::get_type_at_addr(uintptr_t addr) {
	PhysicalMapEntry *entry = m_map;
	while (entry != nullptr) {
		uintptr_t entryBase = entry->physicalBase;
		uintptr_t entryTop = entry->physicalBase + entry->numPages * 0x1000;
		if (addr >= entryBase && addr <= entryTop) { return entry->type; }
		entry = entry->next;
	}
	return INVALID_TYPE;
}

void MdOS::mem::phys::PhysicalMemoryMap::clean_map() {
	PhysicalMapEntry *entry = m_map;

	//delete 0 size entries
	while (entry != nullptr) {
		PhysicalMapEntry *next = entry->next;
		if (entry->numPages == 0) {
			if (entry->prev != nullptr) {
				entry->prev->next = entry->next;
			} else {
				m_map = entry->next;
			}
			if (entry->next != nullptr) { entry->next->prev = entry->prev; }
			free_entry(entry);
		}
		entry = next;
	}

	//remove holes
	entry = m_map;
	while (entry != nullptr && entry->next != nullptr) {
		uintptr_t endAddr = entry->physicalBase + (entry->numPages * 0x1000);
		if (entry->next->physicalBase != endAddr) {
			set_range(endAddr, (entry->next->physicalBase - endAddr) / 0x1000, UNUSABLE_MEMORY);
		}
		entry = entry->next;
	}

	//coaless entries
	entry = m_map;
	while (entry && entry->next) {
		uintptr_t endAddr = entry->physicalBase + entry->numPages * 0x1000;
		PhysicalMapEntry *next = entry->next;
		if (entry->type == next->type && endAddr == next->physicalBase) {
			entry->numPages += next->numPages;
			entry->next = next->next;
			if (next->next) next->next->prev = entry;
			free_entry(next);
		} else {
			entry = entry->next;
		}
	}
}

MdOS::mem::phys::PhysicalMemoryMap::PhysicalMapEntry *MdOS::mem::phys::PhysicalMemoryMap::get_entry() {
	PhysicalMapEntry *newEntry = (PhysicalMapEntry *) m_mapAllocator.allocate_slab();
	if (newEntry != nullptr) { m_numberOfEntries++; }
	return newEntry;
}

void MdOS::mem::phys::PhysicalMemoryMap::free_entry(PhysicalMapEntry *entry) {
	m_mapAllocator.free_slab((void *) entry);
	m_numberOfEntries--;
}