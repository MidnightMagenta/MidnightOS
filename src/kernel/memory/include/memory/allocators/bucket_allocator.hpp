#ifndef MDOS_BUCKET_ALLOCATOR_H
#define MDOS_BUCKET_ALLOCATOR_H

#include <k_utils/result.h>
#include <stddef.h>

namespace MdOS::mem {
    Result bucket_heap_init();
    void* _internal_alloc(size_t size);
    void _internal_free(void* addr);
}// namespace MdOS::mem

#endif