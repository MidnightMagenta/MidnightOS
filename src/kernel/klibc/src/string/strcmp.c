#include <klibc/string.h>

int strcmp(const char *str1, const char *str2) {
	size_t i = 0;
	size_t res = 0;
	while ((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0')) { i++; }
	res = ((unsigned char) str1[i] - (unsigned char) str2[i]);
	return (int) res;
}

int strncmp(const char* str1, const char* str2, size_t num){
    size_t i = 0;
    size_t res = 0;
    while((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0') && (i < num)){
        i++;
    }
    res = ((unsigned char) str1[i] - (unsigned char) str2[i]);
    return (int) res;
}