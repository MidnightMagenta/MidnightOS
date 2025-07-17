#ifndef MDOS_BUCKET_ALLOCATOR_H
#define MDOS_BUCKET_ALLOCATOR_H

#include <k_utils/result.h>
#include <stddef.h>
#include <stdint.h>

#define MIN_HEAP_ORDER 3
#define MAX_HEAP_ORDER 11

namespace MdOS::mem {
Result bucket_heap_init();
void *_internal_alloc(size_t size);
bool _internal_free(void *addr);

void _internal_set_bucket_alloc_avail();
inline int fast_u64_ceil_log2(uint64_t value) { return 64 - __builtin_clzll(value - 1); }
}// namespace MdOS::mem

#endif