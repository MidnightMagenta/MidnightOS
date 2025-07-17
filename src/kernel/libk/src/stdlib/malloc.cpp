#include <boot/kernel_status.h>
#include <libk/stdlib.h>
#include <memory/allocators/bucket_allocator.hpp>
#include <memory/allocators/bump_allocator.hpp>

extern "C" void *malloc(size_t size) {
	if (mdos_get_status_flag(MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL) && MdOS::mem::fast_u64_ceil_log2(size) <= MAX_HEAP_ORDER) {
		return MdOS::mem::_internal_alloc(size);
	} else if (mdos_get_status_flag(MDOS_STATUS_FLAG_VMALLOC_AVAIL)) {
		/*void*/
	}

	return MdOS::mem::g_bumpAlloc->alloc(size);
}

extern "C" void *malloca(size_t size, size_t alignment) {
	if (mdos_get_status_flag(MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL) && MdOS::mem::fast_u64_ceil_log2(size) <= MAX_HEAP_ORDER) {
		return MdOS::mem::_internal_alloc(size);
	} else if (mdos_get_status_flag(MDOS_STATUS_FLAG_VMALLOC_AVAIL)) {
		/*void*/
	}

	return MdOS::mem::g_bumpAlloc->alloc_aligned(size, alignment);
}

extern "C" void free(void *addr) {
	if (mdos_get_status_flag(MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL) && MdOS::mem::_internal_free(addr)) { return; }
	if (mdos_get_status_flag(MDOS_STATUS_FLAG_VMALLOC_AVAIL)) { return; }
}