#ifndef MDOS_LIBK_STDLIB_H
#define MDOS_LIBK_STDLIB_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void* malloc(size_t size);
void* malloca(size_t size, size_t alignment);
void free(void* addr);

#ifdef __cplusplus
}
#endif
#endif