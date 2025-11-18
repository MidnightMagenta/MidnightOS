#include <string.h>

void *memcpy(void *restrict dst, const void *restrict src, size_t num) {
    unsigned char       *dstPtr = (unsigned char *) dst;
    const unsigned char *srcPtr = (const unsigned char *) src;

    while (num--) { *dstPtr++ = *srcPtr++; }
    return dst;
}
