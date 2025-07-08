#ifndef MDOS_PHYSICAL_MEM_MAP_H
#define MDOS_PHYSICAL_MEM_MAP_H

#include <k_utils/memory.h>
#include <k_utils/result.hpp>
#include <memory/pmm.hpp>

namespace MdOS::memory::PMM {
enum MemoryType {
	FREE_MEMORY,
	KERNEL_RESERVED_MEMORY,
	EFI_RESERVED_MEMORY,
	UNUSABLE_MEMORY,
	KERNEL_ALLOCATED_MEMORY,
	DRIVER_ALLOCATED_MEMORY,
	USERSPACE_ALLOCATED_MEMORY,
};

struct PhysicalMapEntry {
	uintptr_t physicalBase;
	size_t numPages;
	uint32_t type;
};


}// namespace MdOS::Memory::PMM

#endif