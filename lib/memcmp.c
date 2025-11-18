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
