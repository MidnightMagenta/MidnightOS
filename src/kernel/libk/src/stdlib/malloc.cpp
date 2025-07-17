#include <libk/stdlib.h>
#include <memory/allocators/bucket_allocator.hpp>
#include <memory/allocators/bump_allocator.hpp>

bool bucketAllocatorAvail = false;
bool vmallocAvail = false;

void MdOS::mem::_internal_set_bucket_alloc_avail() { bucketAllocatorAvail = true; }

extern "C" void *malloc(size_t size) {
	if (bucketAllocatorAvail && MdOS::mem::fast_u64_ceil_log2(size) <= MAX_HEAP_ORDER) {
		return MdOS::mem::_internal_alloc(size);
	} else if (vmallocAvail) {
		/*void*/
	}

	return MdOS::mem::g_bumpAlloc->alloc(size);
}

void *malloca(size_t size, size_t alignment) {
	if (bucketAllocatorAvail && MdOS::mem::fast_u64_ceil_log2(size) <= MAX_HEAP_ORDER) {
		return MdOS::mem::_internal_alloc(size);
	} else if (vmallocAvail) {
		/*void*/
	}

	return MdOS::mem::g_bumpAlloc->alloc_aligned(size, alignment);
}

void free(void *addr) {
	if (bucketAllocatorAvail && MdOS::mem::_internal_free(addr)) { return; }
	if (vmallocAvail) { return; }
}