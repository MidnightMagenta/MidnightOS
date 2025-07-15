#ifndef MDOS_KBLIC_STDLIB_H
#define MDOS_KBLIC_STDLIB_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void* malloc(size_t size);
void* malloca(size_t size, size_t alignment);

#ifdef __cplusplus
}
#endif
#endif