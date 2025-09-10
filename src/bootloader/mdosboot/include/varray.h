#ifndef MDOSBOOT_VARRAY_H
#define MDOSBOOT_VARRAY_H

#include <efi.h>
#include <efilib.h>

typedef struct {
	void *data;
	UINTN entrySize;
	UINTN size;
	UINTN capacity;
} MDOSBOOT_VARRAY;

EFI_STATUS varray_initialize(OUT MDOSBOOT_VARRAY *array, IN UINTN dataSize, IN UINTN initialCapacity);
EFI_STATUS varray_free(IN MDOSBOOT_VARRAY *array);
EFI_STATUS varray_push_back(IN MDOSBOOT_VARRAY *array, IN void *data);
EFI_STATUS varray_pop_back(IN MDOSBOOT_VARRAY *array, OUT void *data);

#endif