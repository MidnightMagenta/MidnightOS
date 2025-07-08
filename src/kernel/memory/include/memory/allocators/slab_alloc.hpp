#ifndef MDOS_SLAB_ALLOCATOR_H
#define MDOS_SLAB_ALLOCATOR_H

#include <stddef.h>

namespace MdOS::Memory::allocators {
class SlabAllocator {
public:
    SlabAllocator(){}
    SlabAllocator(size_t size, size_t slabSize);
    SlabAllocator(void* allocBase, size_t size, size_t slabSize);
    ~SlabAllocator(){}
private:
    void* m_allocBase = nullptr;
    size_t m_slabSize = 0;
};
}// namespace MdOS::Memory::allocators

#endif