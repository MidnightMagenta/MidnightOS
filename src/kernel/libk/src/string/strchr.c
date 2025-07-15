#include <libk/string.h>

char *strchr(const char *str, int c) {
	char *res = NULL;
	while ((*str != '\0') && (*str != c)) { str++; }
	if (*str == c) { res = (char *) str; }
	return res;
}