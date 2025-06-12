#ifndef BOOT_EFI_MAP_H
#define BOOT_EFI_MAP_H

#include "../include/basics.h"
#include "../include/types.h"
#include <efi.h>
#include <efilib.h>

size_t get_efi_map_size(EFI_SYSTEM_TABLE *systemTable);
EFI_STATUS get_EFI_map_noalloc(EFI_SYSTEM_TABLE *systemTable, MemMap *map);
EFI_STATUS get_EFI_map(EFI_SYSTEM_TABLE *systemTable, MemMap *map);

#endif