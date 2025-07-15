#include <libk/string.h>

void *memset(void *ptr, int val, size_t num) {
	unsigned char *strPtr = (unsigned char *) ptr;
	while (num--) { *strPtr++ = (unsigned char) val; }
    return ptr;
}