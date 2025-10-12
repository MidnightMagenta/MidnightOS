#include "../include/types.h"

#define EFI_MAP_DESCRIPTOR_EXTRA 8192

EFI_STATUS efi_get_memmap(efi_memmap_t *const map, size_t *bufferSize) {
	EFI_STATUS res;
	if (map == NULL) { return EFI_INVALID_PARAMETER; }

	res = gBS->GetMemoryMap(&map->size, NULL, &map->key, &map->descSize, &map->descVersion);
	if (res != EFI_BUFFER_TOO_SMALL) { return res; }

	map->size += EFI_MAP_DESCRIPTOR_EXTRA;
	*bufferSize = map->size;
	res = gBS->AllocatePool(EfiLoaderData, map->size, (void **) &map->descs);
	if (EFI_ERROR(res)) { return res; }

	res = gBS->GetMemoryMap(&map->size, map->descs, &map->key, &map->descSize, &map->descVersion);
	if (EFI_ERROR(res)) {
		gBS->FreePool(map->descs);
		return res;
	}

	return EFI_SUCCESS;
}

EFI_STATUS efi_get_memmap_norealloc(size_t bufferSize, efi_memmap_t *const map) {
	EFI_STATUS res;

	map->size = bufferSize;
	res = gBS->GetMemoryMap(&map->size, map->descs, &map->key, &map->descSize, &map->descVersion);
	if (EFI_ERROR(res)) { return res; }

	return EFI_SUCCESS;
}

void efi_free_memmap(efi_memmap_t *const map) {
	gBS->FreePool(map->descs);
	map->size = 0;
	map->key = 0;
	map->descSize = 0;
	map->descVersion = 0;
}