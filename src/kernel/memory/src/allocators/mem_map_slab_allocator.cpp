#include <IO/debug_print.h>
#include <k_utils/utils.hpp>
#include <memory/allocators/bump_allocator.hpp>
#include <memory/allocators/mem_map_slab_allocator.hpp>

void MdOS::mem::MemMapSlabAllocator::init(void *allocBase, size_t size, size_t slabSize,
														 Allocator *allocator) {
	kassert(size > slabSize);
	size = ALIGN_DOWN(size, slabSize, uintptr_t);
	m_slabCount = size / slabSize;
	m_freeSlabs = m_slabCount;
	m_slabSize = slabSize;
	m_freeList = (Slab *) allocator->alloc_aligned(m_slabCount * sizeof(Slab),
												   sizeof(Slab));//TODO: replace with proper kmalloc

	uintptr_t addr = uintptr_t(allocBase);
	Slab *prev = nullptr;
	Slab *current = m_freeList;
	for (size_t i = 0; i < m_slabCount; ++i) {
		current->prev = prev;
		current->addr = addr;

		if (i < m_slabCount - 1) {
			current->next = current + 1;
		} else {
			current->next = nullptr;
		}

		prev = current;
		current = current->next;
		addr += slabSize;
	}
}

void *MdOS::mem::MemMapSlabAllocator::allocate_slab() {
	if (!m_freeList) { return nullptr; }
	Slab *allocation = m_freeList;
	m_freeList = allocation->next;
	if (m_freeList != nullptr) { m_freeList->prev = nullptr; }

	allocation->next = nullptr;
	allocation->prev = nullptr;

	allocation->next = m_usedList;
	if (m_usedList != nullptr) { m_usedList->prev = allocation; }
	allocation->prev = nullptr;
	m_usedList = allocation;

	m_freeSlabs--;
	m_usedSlabs++;

	return (void *) allocation->addr;
}

void MdOS::mem::MemMapSlabAllocator::free_slab(void *addr) {
	Slab *current = m_usedList;
	while (current != nullptr) {
		if (current->addr == uintptr_t(addr)) {
			if (current->prev != nullptr) { current->prev->next = current->next; }
			if (current->next != nullptr) { current->next->prev = current->prev; }
			if (current == m_usedList) { m_usedList = current->next; }

			current->next = nullptr;
			current->prev = nullptr;

			current->prev = nullptr;
			current->next = m_freeList;
			if (m_freeList != nullptr) m_freeList->prev = current;
			m_freeList = current;

			m_freeSlabs++;
			m_usedSlabs--;

			return;
		}
		current = current->next;
	}

	current = m_freeList;
	while (current != nullptr) {
		if (current->addr == uintptr_t(addr)) {
			DEBUG_LOG("Attempted to deallocate a free address\n");
			return;
		}
		current = current->next;
	}
	PRINT_ERROR("Attempted to deallocate an invalid address");
}

bool MdOS::mem::MemMapSlabAllocator::verify_allocator_coherency() {
	Slab *current = m_freeList;
	size_t slabCount = 0;
	while (current != nullptr) {
		slabCount++;
		current = current->next;
	}
	if (slabCount != m_freeSlabs) { return false; }

	current = m_usedList;
	slabCount = 0;
	while (current != nullptr) {
		slabCount++;
		current = current->next;
	}
	if (slabCount != m_usedSlabs) { return false; }

	return true;
}

void MdOS::mem::MemMapSlabAllocator::fix_allocator_coherency() {
	if (verify_allocator_coherency()) { return; }

	Slab *current = m_freeList;
	size_t freeSlabCount = 0;
	while (current != nullptr) {
		freeSlabCount++;
		current = current->next;
	}
	m_freeSlabs = freeSlabCount;

	current = m_usedList;
	size_t usedSlabCount = 0;
	while (current != nullptr) {
		usedSlabCount++;
		current = current->next;
	}
	m_usedSlabs = usedSlabCount;
	m_slabCount = m_freeSlabs + m_usedSlabs;
}