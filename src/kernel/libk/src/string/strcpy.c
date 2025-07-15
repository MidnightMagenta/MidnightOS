#include <libk/string.h>

char *strcpy(char *dst, const char *src) {
	int i = 0;
	while (src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t num) {
	char *dstPtr = dst;
	const char *srcPtr = src;
	while (num--) { *dstPtr++ = *srcPtr++; }
	return dst;
}