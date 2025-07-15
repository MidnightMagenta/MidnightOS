#include <libk/stdlib.h>
#include <memory/allocators/bump_allocator.hpp>

bool mallocReady = false;

extern "C" void *malloc(size_t size) {
	// TODO: implement proper allocator usage
	if (!mallocReady) { return MdOS::mem::g_bumpAlloc->alloc(size); }
	return nullptr;
}

void *malloca(size_t size, size_t alignment) {
	// TODO: implement proper allocator usage
	if (!mallocReady) { return MdOS::mem::g_bumpAlloc->alloc_aligned(size, alignment); }
	return nullptr;
}

void free(void *addr) {
	// TODO: implement proper allocator usage
	if (!mallocReady) { MdOS::mem::g_bumpAlloc->free(addr); }
}