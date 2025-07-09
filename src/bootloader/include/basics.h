#ifndef MDBOOT_BOOT_BASICS_H
#define MDBOOT_BOOT_BASICS_H

#include <efi.h>
#include <efilib.h>

#define VERBOSE_REPORTING 0

#define HandleError(fmt, status)                                                                                       \
	if (status != EFI_SUCCESS) {                                                                                       \
		Print(L"Critical error: " fmt L": 0x%lx\n\r", status);                                                         \
		return status;                                                                                                 \
	}

#define ALIGN_ADDR(val, alignment, castType)                                                                           \
	((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))

int mem_compare(const void *aptr, const void *bptr, size_t n);

#endif