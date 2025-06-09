#ifndef BUMP_ALLOCATOR_H
#define BUMP_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

namespace MdOS::Memory {
class BumpAllocator {
public:
	BumpAllocator() {}
	~BumpAllocator() {}

	void init(uint64_t *heapBase, uint64_t *heapTop);
	void init(uint64_t *heapBase, uint64_t heapSize);

	void *alloc(size_t size);

private:
	uint64_t *m_heapBase;
	uint64_t *m_heapTop;
	uint64_t *m_allocPtr;
};

static BumpAllocator g_bumpAlloc;
}// namespace MdOS::Memory

#endif