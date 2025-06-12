#ifndef BOOT_BASICS_H
#define BOOT_BASICS_H

#include <efi.h>
#include <efilib.h>

#define VERBOSE_REPORTING 1

#define HandleError(fmt, status)                                                                                       \
	if (status != EFI_SUCCESS) {                                                                                       \
		Print(L"Critical error: " fmt L": 0x%lx\n\r", status);                                                         \
		return status;                                                                                                 \
	}

#define ALIGN_ADDR(val, alignment, castType)                                                                           \
	((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))

EFI_STATUS open_file(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable,
					 EFI_FILE **file);
int mem_compare(const void *aptr, const void *bptr, size_t n);

#endif