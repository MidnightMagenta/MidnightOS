#include <IO/debug_print.h>
#include <memory/allocators/bump_allocator.hpp>

void MdOS::mem::BumpAllocator::init(uintptr_t heapBase, uintptr_t heapTop) {
	m_heapBase = heapBase;
	m_heapTop = heapTop;
	m_allocPtr = heapBase;
}

void *MdOS::mem::BumpAllocator::alloc(size_t size) {
	if (m_allocPtr == 0 || m_heapBase == 0 || m_heapTop == 0) {
		PRINT_ERROR("uninitialized");
		return nullptr;
	}
	if (m_allocPtr + size >= m_heapTop) {
		PRINT_ERROR("out of memory");
		return nullptr;
	}

	uintptr_t allocAddress = m_allocPtr;
	m_allocPtr = m_allocPtr + size;

	ALLOC_LOG("Allocated %lu bytes at address 0x%lx", size, allocAddress);
	return (void *) allocAddress;
}

void *MdOS::mem::BumpAllocator::alloc_aligned(size_t size, size_t alignment) {
	uintptr_t allocAddress = ALIGN_ADDR(m_allocPtr, alignment, uintptr_t);

	if (m_allocPtr == 0 || m_heapBase == 0 || m_heapTop == 0) {
		PRINT_ERROR("uninitialized");
		return nullptr;
	}
	if (allocAddress + size >= m_heapTop) {
		PRINT_ERROR("out of memory");
		return nullptr;
	}

	m_allocPtr = allocAddress + size;
	
	ALLOC_LOG("Allocated %lu bytes at address 0x%lx", size, allocAddress);
	return (void *) allocAddress;
}