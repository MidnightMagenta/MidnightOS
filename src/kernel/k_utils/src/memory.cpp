#include <k_utils/memory.hpp>

void *utils::memset(void *ptr, int value, size_t num) {
	for (byte_t *mem = (byte_t *) ptr; mem < (byte_t *) (uintptr_t(ptr) + num); mem++) { *mem = byte_t(value); }
	return ptr;
}