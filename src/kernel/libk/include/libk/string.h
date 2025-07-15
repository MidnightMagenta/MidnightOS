#ifndef MDOS_libk_STRING_H
#define MDOS_libk_STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t num);

char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t num);

int memcmp(const void *ptr1, const void *ptr2, size_t num);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);

void *memchr(const void *str, int val, size_t num);
char *strchr(const char *str, int c);
size_t strspn(const char *s, const char *accept);
char *strpbrk(const char *str, const char *accept);
char *strstr(const char *str1, const char *str2);
char *strtok(char *str, const char *delim);

void *memset(void *ptr, int val, size_t num);
size_t strlen(const char *str);

#ifdef __cplusplus
}
#endif
#endif