#include <klibc/string.h>

void *memchr(const void *str, int val, size_t num) {
	const unsigned char *ptr = (const unsigned char *) str;
	while (num--) {
		if (*ptr == (unsigned char) val) { return (void *) ptr; }
		ptr++;
	}
	return NULL;
}