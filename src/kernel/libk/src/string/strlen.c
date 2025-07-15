#include <libk/string.h>

size_t strlen(const char *str) {
	size_t len = 0;
	if (str) {
		while (str[len] != '\0') { len++; }
	}
	return len;
}