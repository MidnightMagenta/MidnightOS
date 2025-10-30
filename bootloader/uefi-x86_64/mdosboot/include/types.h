#ifndef MDOSBOOT_TYPES_H
#define MDOSBOOT_TYPES_H

#include <efi.h>
#include <efilib.h>

typedef struct {
  EFI_MEMORY_DESCRIPTOR *descs;
  UINTN size;
  UINTN key;
  UINTN descSize;
  UINT32 descVersion;
} efi_memmap_t;

EFI_STATUS efi_get_memmap(efi_memmap_t *const map, size_t *bufferSize);
EFI_STATUS efi_get_memmap_norealloc(size_t bufferSize, efi_memmap_t *const map);
void efi_free_memmap(efi_memmap_t *const map);

#endif