#include <libk/string.h>

char *strpbrk(const char *str, const char *accept) {
	while (*str != '\0') {
		const char *a = accept;
		while (*a != '\0') {
			if (*a++ == *str) { return (char *) str; }
		}
		str++;
	}
	return NULL;
}
