#include <klibc/string.h>

char *strstr(const char *str1, const char *str2) {
	size_t str2_len = strlen(str2);
	if (str2_len == 0) { return (char *) str1; }
	while (*str1) {
		if (*str1 == *str2 && strncmp(str1, str2, str2_len)) { return (char *) str1; }
		str1++;
	}
	return NULL;
}