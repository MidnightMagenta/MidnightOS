#include "../include/varray.h"
#include "../include/debug.h"

EFI_STATUS varray_initialize(OUT MDOSBOOT_VARRAY *array, IN UINTN dataSize, IN UINTN initialCapacity) {
	if (array == NULL || dataSize == 0) { return EFI_INVALID_PARAMETER; }

	EFI_STATUS res = gBS->AllocatePool(EfiLoaderData, initialCapacity * dataSize, (void **) &array->data);
	if (EFI_ERROR(res)) {
		DBG_MSG("[%a %d] Failed to allocate config array memory with: %lx\n\r", __func__, __LINE__, res);
		return res;
	}

	array->entrySize = dataSize;
	array->capacity = initialCapacity;
	array->size = 0;
	return EFI_SUCCESS;
}

EFI_STATUS varray_free(IN MDOSBOOT_VARRAY *array) {
	if (array == NULL) { return EFI_INVALID_PARAMETER; }

	gBS->FreePool(array->data);
	array->entrySize = 0;
	array->capacity = 0;
	array->size = 0;
	return EFI_SUCCESS;
}

EFI_STATUS varray_push_back(IN MDOSBOOT_VARRAY *array, IN void *data) {
	if (array == NULL || data == NULL) { return EFI_INVALID_PARAMETER; }

	EFI_STATUS res;
	if (array->size >= array->capacity) {
		void *temp = NULL;
		res = gBS->AllocatePool(EfiLoaderData, (array->capacity * 2) * array->entrySize, (void **) &temp);
		if (EFI_ERROR(res)) { return res; }
		CopyMem(temp, array->data, array->capacity * array->entrySize);

		gBS->FreePool(array->data);
		array->data = temp;
		array->capacity *= 2;
	}

	CopyMem((void *) ((char *) array->data + (array->size * array->entrySize)), data, array->entrySize);
	array->size++;
	return EFI_SUCCESS;
}

EFI_STATUS varray_pop_back(IN MDOSBOOT_VARRAY *array, OUT void *data) {
	if (array == NULL || data == NULL) { return EFI_INVALID_PARAMETER; }
	if (array->size == 0) { return EFI_NOT_READY; }

	CopyMem(data, (void *) ((char *) array->data + ((array->size - 1) * array->entrySize)), array->entrySize);
	array->size--;
	return EFI_SUCCESS;
}