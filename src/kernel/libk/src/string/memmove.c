#include <libk/string.h>

void *memmove(void *dst, const void *src, size_t num) {
	unsigned char *dstPtr = (unsigned char *) dst;
	const unsigned char *srcPtr = (const unsigned char *) src;

	if (dstPtr < srcPtr) {
		while (num--) { *dstPtr++ = *srcPtr++; }
	} else {
		dstPtr += num - 1;
		srcPtr += num - 1;
		while (num--) { *dstPtr-- = *srcPtr--; }
	}
	return dst;
}
