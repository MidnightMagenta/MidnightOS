#include "../include/bump_allocator.hpp"
#include "../../IO/include/debug_print.hpp"

uintptr_t MdOS::Memory::BumpAllocator::m_heapBase = 0;
uintptr_t MdOS::Memory::BumpAllocator::m_heapTop = 0;
uintptr_t MdOS::Memory::BumpAllocator::m_allocPtr = 0;

void MdOS::Memory::BumpAllocator::init(uintptr_t heapBase, uintptr_t heapTop) {
	m_heapBase = heapBase;
	m_heapTop = heapTop;
	m_allocPtr = heapBase;
}

void *MdOS::Memory::BumpAllocator::alloc(size_t size) {
	if (m_allocPtr == 0 || m_heapBase == 0 || m_heapTop == 0) {
		PRINT_ERROR("uninitialized");
		return nullptr;
	}
	if (m_allocPtr + uintptr_t(size) >= m_heapTop) {
		PRINT_ERROR("out of memory");
		return nullptr;
	}

	uintptr_t allocAddress = m_allocPtr;
	m_allocPtr = m_allocPtr + uintptr_t(size);
	return (void *) allocAddress;
}

void *MdOS::Memory::BumpAllocator::aligned_alloc(size_t size, size_t alignment) {
	uintptr_t allocAddress = ALIGN_ADDR(m_allocPtr, alignment, uintptr_t);

	if (m_allocPtr == 0 || m_heapBase == 0 || m_heapTop == 0) {
		PRINT_ERROR("uninitialized");
		return nullptr;
	}
	if (allocAddress + uintptr_t(size) >= m_heapTop) {
		PRINT_ERROR("out of memory");
		return nullptr;
	}

	m_allocPtr = allocAddress + uintptr_t(size);
	return (void *) allocAddress;
}
