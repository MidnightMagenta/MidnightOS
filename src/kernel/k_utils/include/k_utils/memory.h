#ifndef MDOS_K_UTILS_MEMORY_H
#define MDOS_K_UTILS_MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <k_utils/types.h>
#include <stddef.h>
#include <stdint.h>

void *memset(void *ptr, int value, size_t count);
void *memcpy(void *dst, void *src, size_t count);

#ifdef __cplusplus
}
#endif
#endif