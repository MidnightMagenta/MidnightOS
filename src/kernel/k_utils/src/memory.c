#include <k_utils/memory.h>

void *memset(void *ptr, int value, size_t count) {
	for (byte_t *mem = (byte_t *) ptr; mem < (byte_t *) ((uintptr_t) ptr + count); mem++) { *mem = (byte_t) value; }
	return ptr;
}

void *memcpy(void *dst, void *src, size_t count) {
	byte_t *dstPtr = (byte_t *) dst;
	byte_t *srcPtr = (byte_t *) src;
	byte_t *endPtr = (byte_t *) ((uintptr_t) dstPtr + count);

	while (dstPtr < endPtr) {
		*dstPtr = *srcPtr;
		dstPtr++;
		srcPtr++;
	}
	return dst;
}