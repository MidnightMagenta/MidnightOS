#include <klibc/string.h>

char *strcat(char *dst, const char *src) {
	size_t last = strlen(dst);
	size_t i = 0;
	while (src[i] != '\0') {
		dst[last] = src[i];
		i++;
		last++;
	}
	dst[last] = '\0';
	return dst;
}

char *strncat(char *dst, const char *src, size_t num) {
	size_t last = strlen(dst);
	size_t i = 0;
	while (i < num) {
		dst[last] = src[i];
		last++;
		i++;
	}
	dst[last] = '\0';
	return dst;
}