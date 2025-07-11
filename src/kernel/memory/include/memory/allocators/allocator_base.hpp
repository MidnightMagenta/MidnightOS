#ifndef MDOS_ALLOCATOR_BASE_H
#define MDOS_ALLOCATOR_BASE_H

#include <stddef.h>

namespace MdOS::mem {
class Allocator {
public:
	Allocator() {}
	~Allocator() {}

	virtual void *alloc(size_t size) = 0;
	virtual void *alloc_aligned(size_t size, size_t alignment) = 0;
	virtual void free(void *addr) = 0;
};
}// namespace MdOS::mem::allocators


#endif