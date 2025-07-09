#include "../include/efi_map.h"

size_t get_efi_map_size(EFI_SYSTEM_TABLE *systemTable) {
	UINTN size = 0;
	UINTN key;
	UINTN descriptorSize;
	UINT32 descriptorVersion;
	systemTable->BootServices->GetMemoryMap(&size, NULL, &key, &descriptorSize, &descriptorVersion);
	return size;
}

EFI_STATUS get_EFI_map_noalloc(EFI_SYSTEM_TABLE *systemTable, MemMap *map) {
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize,
																&map->descriptorVersion);
	return status;
}

EFI_STATUS get_EFI_map(EFI_SYSTEM_TABLE *systemTable, MemMap *map) {
	map->size = get_efi_map_size(systemTable) + (100 * sizeof(EFI_MEMORY_DESCRIPTOR));
	EFI_STATUS status = systemTable->BootServices->AllocatePool(EfiLoaderData, map->size, (void **) &map->map);
	if (status != EFI_SUCCESS) { return status; }
	status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize,
													 &map->descriptorVersion);
	return status;
}