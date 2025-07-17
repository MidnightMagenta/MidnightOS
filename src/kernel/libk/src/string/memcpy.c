#include <libk/string.h>

void *memcpy(void *dst, const void *src, size_t num) {
	unsigned char *dstPtr = (unsigned char *) dst;
	const unsigned char *srcPtr = (const unsigned char *) src;

	while (num--) { *dstPtr++ = *srcPtr++; }
	return dst;
}
