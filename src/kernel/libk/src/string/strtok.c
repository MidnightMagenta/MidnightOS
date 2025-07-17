#include <libk/string.h>

char *oldStr;

char *strtok(char *str, const char *delim) {
	char *token = NULL;
	if (str == NULL) { str = oldStr; }

	str += strspn(str, delim);
	if (*str == '\0') {
		oldStr = str;
		return NULL;
	}

	token = str;
	str = strpbrk(token, delim);
	if (str == NULL) {
		oldStr = (char*) memchr(token, '\0', 1024);
	} else {
		*str = '\0';
		oldStr = str + 1;
	}
	return token;
}
