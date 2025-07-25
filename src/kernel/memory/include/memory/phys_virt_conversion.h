#ifndef MDOS_PHYS_VIRT_CONVERSION_H
#define MDOS_PHYS_VIRT_CONVERSION_H
#ifdef __cpluplus
extern "C" {
#endif

#define MDOS_MEMORY_DIRECT_MAP_REGION_BASE 0xFFFF800000000000ULL
#define MDOS_MEMORY_DIRECT_MAP_REGION_END 0xFFFF880000000000ULL
#define MDOS_VIRT_TO_PHYS(vaddr) (vaddr - MDOS_MEMORY_DIRECT_MAP_REGION_BASE)
#define MDOS_PHYS_TO_VIRT(paddr) (paddr + MDOS_MEMORY_DIRECT_MAP_REGION_BASE)

#ifdef __cpluplus
}
#endif
#endif