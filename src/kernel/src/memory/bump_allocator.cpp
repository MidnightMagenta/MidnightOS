#include "../../include/memory/bump_allocator.hpp"

void MdOS::Memory::BumpAllocator::Init(uint64_t *heapBase, uint64_t *heapTop) {
    m_heapBase = heapBase;
    m_heapTop = heapTop;
    m_allocPtr = heapBase;
}

void MdOS::Memory::BumpAllocator::Init(uint64_t *heapBase, uint64_t heapSize) {
    m_heapBase = heapBase;
    m_heapTop = (uint64_t*)(uint64_t(heapBase) + heapSize);
    m_allocPtr = heapBase;
}

void *MdOS::Memory::BumpAllocator::alloc(size_t size) { 
    uint64_t* allocAddress = m_allocPtr;
    m_allocPtr = (uint64_t*)(uint64_t(m_allocPtr) + size);
    return allocAddress;
}
