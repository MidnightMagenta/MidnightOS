#ifndef MDOS_BUMP_ALLOCATOR_H
#define MDOS_BUMP_ALLOCATOR_H

#include <k_utils/utils.hpp>
#include <memory/allocators/allocator_base.hpp>
#include <k_utils/compiler_options.h>
#include <stddef.h>
#include <stdint.h>

namespace MdOS::mem {
class BumpAllocator : public Allocator {
public:
	BumpAllocator() {}
	~BumpAllocator() {}
	void init(uintptr_t heapBase, uintptr_t heapTop);

	void *alloc(size_t size) override;
	void *alloc_aligned(size_t size, size_t alignment) override;
	void free(MDOS_UNUSED void *addr) override {}

private:
	uintptr_t m_heapBase = 0;
	uintptr_t m_heapTop = 0;
	uintptr_t m_allocPtr = 0;
};

inline BumpAllocator *g_bumpAlloc = nullptr;
}// namespace MdOS::memory::allocators

#endif