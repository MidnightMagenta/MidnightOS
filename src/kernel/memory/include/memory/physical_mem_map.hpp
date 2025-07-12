#ifndef MDOS_PHYSICAL_MEM_MAP_H
#define MDOS_PHYSICAL_MEM_MAP_H

#include <k_utils/result.hpp>
#include <memory/allocators/mem_map_slab_allocator.hpp>
#include <k_utils/compiler_options.h>

namespace MdOS::mem::phys {
enum MemoryType {
	FREE_MEMORY,
	KERNEL_RESERVED_MEMORY,
	EFI_RESERVED_MEMORY,
	EFI_ACPI_RECLAIMABLE_MEMORY,
	EFI_RECLAIMABLE_MEMORY,
	KERNEL_ALLOCATED_MEMORY,
	DRIVER_ALLOCATED_MEMORY,
	USERSPACE_ALLOCATED_MEMORY,
	UNUSABLE_MEMORY,
	INVALID_TYPE,
};

struct PhysicalMemoryDescriptor {
	uintptr_t baseAddr = 0;
	size_t numPages = 0;
	uint32_t type = INVALID_TYPE;
};

class PhysicalMemoryMap {
public:
	PhysicalMemoryMap();
	~PhysicalMemoryMap();

	struct PhysicalMapEntry {
		uintptr_t physicalBase = 0;
		size_t numPages = 0;
		uint32_t type = FREE_MEMORY;
		char pad[4];

		PhysicalMapEntry *next = nullptr;
		PhysicalMapEntry *prev = nullptr;
	} MDOS_PACKED MDOS_ALIGNED(8);

	void set_range(uintptr_t addr, size_t numPages, uint32_t type);
	void print_map();
	inline size_t get_number_of_entries() { return m_numberOfEntries; }
	inline uintptr_t get_map_base() { return m_mapBase; }
	inline size_t get_map_size() { return m_mapSize; }

	PhysicalMemoryDescriptor get_first_range(uint32_t type);
	PhysicalMemoryDescriptor get_next_range(uintptr_t addr, uint32_t type);
	PhysicalMemoryDescriptor get_first_fit_range(size_t numPages, uint32_t type);
	uint32_t get_type_at_addr(uintptr_t addr);

	bool initialized() { return m_initialized; }

private:
	void clean_map();
	PhysicalMapEntry *get_entry();
	void free_entry(PhysicalMapEntry *entry);

	PhysicalMapEntry *m_map = nullptr;
	size_t m_numberOfEntries = 0;
	MdOS::mem::MemMapSlabAllocator m_mapAllocator;

	uintptr_t m_mapBase;
	size_t m_mapSize;
	bool m_initialized = false;
};
}// namespace MdOS::mem::phys

#endif