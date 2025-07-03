#ifndef MDOS_K_UTILS_MEMORY_H
#define MDOS_K_UTILS_MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <k_utils/types.h>
#include <stddef.h>
#include <stdint.h>

#define MDOS_MEMORY_DIRECT_MAP_REGION_BASE 0xFFFF800000000000ULL
#define MDOS_MEMORY_DIRECT_MAP_REGION_END 0xFFFF880000000000ULL
#define MDOS_VIRT_TO_PHYS(vaddr) (vaddr - MDOS_MEMORY_DIRECT_MAP_REGION_BASE)
#define MDOS_PHYS_TO_VIRT(paddr) (paddr + MDOS_MEMORY_DIRECT_MAP_REGION_BASE)

void *memset(void *ptr, int value, size_t count);
void *memcpy(void *dst, void *src, size_t count);

#ifdef __cplusplus
}
#endif
#endif