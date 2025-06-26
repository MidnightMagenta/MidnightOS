#ifndef MDOS_BUMP_ALLOCATOR_H
#define MDOS_BUMP_ALLOCATOR_H

#include <k_utils/utils.hpp>
#include <stddef.h>
#include <stdint.h>

namespace MdOS::Memory {
class BumpAllocator {
public:
	static void init(uintptr_t heapBase, uintptr_t heapTop);

	static void *alloc(size_t size);
	static void *aligned_alloc(size_t size, size_t alignment);
	static void *alloc_pages(size_t numPages);

private:
	static uintptr_t m_heapBase;
	static uintptr_t m_heapTop;
	static uintptr_t m_allocPtr;
};
}// namespace MdOS::Memory

#endif