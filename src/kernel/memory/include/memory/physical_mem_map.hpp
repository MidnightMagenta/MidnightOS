#ifndef MDOS_PHYSICAL_MEM_MAP_H
#define MDOS_PHYSICAL_MEM_MAP_H

#include <k_utils/result.h>
#include <memory/allocators/mem_map_slab_allocator.hpp>
#include <k_utils/compiler_options.h>
#include <memory/memory_types.hpp>
#include <thread/spinlock.hpp>

namespace MdOS::mem::phys {

struct PhysicalMemoryDescriptor {
	uintptr_t baseAddr = 0;
	size_t numPages = 0;
	uint8_t type = INVALID_TYPE;
};

class PhysicalMemoryMap {
public:
	PhysicalMemoryMap();
	~PhysicalMemoryMap();

	struct PhysicalMapEntry {
		uintptr_t physicalBase = 0;
		size_t numPages = 0;
		uint8_t type = FREE_MEMORY;
		char pad[7];

		PhysicalMapEntry *next = nullptr;
		PhysicalMapEntry *prev = nullptr;
	} MDOS_PACKED MDOS_ALIGNED(8);

	void set_range(uintptr_t addr, size_t numPages, uint8_t type);
	void print_map();
	inline size_t get_number_of_entries() { return m_numberOfEntries; }
	inline uintptr_t get_map_base() { return m_mapBase; }
	inline size_t get_map_size() { return m_mapSize; }

	PhysicalMemoryDescriptor get_first_range(uint8_t type);
	PhysicalMemoryDescriptor get_next_range(uintptr_t addr, uint8_t type);
	PhysicalMemoryDescriptor get_first_fit_range(size_t numPages, uint8_t type);
	uint8_t get_type_at_addr(uintptr_t addr);

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
	MdOS::thread::Spinlock m_lock;
	bool m_initialized = false;
};
}// namespace MdOS::mem::phys

#endif