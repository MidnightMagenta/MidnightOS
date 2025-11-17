#include <string.h>

int memcmp(const void *a, const void *b, size_t num) {
    const unsigned char *aptr = (const unsigned char *) a;
    const unsigned char *bptr = (const unsigned char *) b;
    while (num--) {
        if (*aptr != *bptr) { return *aptr - *bptr; }
        aptr++;
        bptr++;
    }
    return 0;
}

void *memcpy(void *restrict dst, const void *restrict src, size_t num) {
    unsigned char       *dstPtr = (unsigned char *) dst;
    const unsigned char *srcPtr = (const unsigned char *) src;

    while (num--) { *dstPtr++ = *srcPtr++; }
    return dst;
}

void *memset(void *ptr, int v, size_t num) {
    unsigned char *strPtr = (unsigned char *) ptr;
    while (num--) { *strPtr++ = (unsigned char) v; }
    return ptr;
}

int strcmp(const char *a, const char *b) {
    size_t i   = 0;
    size_t res = 0;
    while ((a[i] == b[i]) && (a[i] != '\0') && (b[i] != '\0')) { i++; }
    res = ((unsigned char) a[i] - (unsigned char) b[i]);
    return (int) res;
}

char *strcpy(char *restrict dst, char *restrict src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

size_t strlen(const char *s) {
    size_t len = 0;
    if (s) {
        while (s[len] != '\0') { len++; }
    }
    return len;
}
