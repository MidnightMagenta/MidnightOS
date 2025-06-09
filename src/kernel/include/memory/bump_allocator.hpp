#ifndef BUMP_ALLOCATOR_H
#define BUMP_ALLOCATOR_H

#include "../../k_utils/include/utils.hpp"
#include <stddef.h>
#include <stdint.h>

namespace MdOS::Memory {
class BumpAllocator {
public:
	BumpAllocator() {}
	~BumpAllocator() {}

	void init(uintptr_t heapBase, uintptr_t heapTop);

	void *alloc(size_t size);
	void *aligned_alloc(size_t size, size_t alignment);

private:
	uintptr_t m_heapBase;
	uintptr_t m_heapTop;
	uintptr_t m_allocPtr;
};

static BumpAllocator g_bumpAlloc;
}// namespace MdOS::Memory

#endif