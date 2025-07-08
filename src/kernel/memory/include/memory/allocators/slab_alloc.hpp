#ifndef MDOS_SLAB_ALLOCATOR_H
#define MDOS_SLAB_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

namespace MdOS::Memory::allocators {
class SlabAllocator : Allocator {
public:
	SlabAllocator() {}
	SlabAllocator(void *allocBase, size_t size, size_t slabSize, Allocator* allocator) { init(allocBase, size, slabSize, allocator); }
	~SlabAllocator() {}

	struct Slab {
		uintptr_t addr = 0;
		Slab *prev = nullptr;
		Slab *next = nullptr;
	};

	void init(void *allocBase, size_t size, size_t slabSize, Allocator* allocator);

	void *allocate_slab();
	void free_slab(void *addr);

	size_t get_free_slab_count() { return m_freeSlabs; }
	size_t get_used_slab_count() { return m_usedSlabs; }
	size_t get_slab_count() { return m_slabCount; }
	size_t get_slab_size() { return m_slabSize; }

    bool verify_allocator_coherency();
    void fix_allocator_coherency(); // fallback function to attempt to recover incoherent memory trackers

private:
	Slab *m_freeList = nullptr;
	Slab *m_usedList = nullptr;

	// memory trackers
	size_t m_slabCount = 0;
	size_t m_freeSlabs = 0;
	size_t m_usedSlabs = 0;
	size_t m_slabSize = 0;
	// !memory trackers
};
}// namespace MdOS::Memory::allocators

#endif