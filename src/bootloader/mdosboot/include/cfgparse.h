#ifndef MDOSBOOT_CFGPARSE_H
#define MDOSBOOT_CFGPARSE_H

#include <efi.h>
#include <efilib.h>

typedef struct {
	EFI_GUID bootPartUUID;
	CHAR16 *bootBinPath;
} ConfigInfo;

EFI_STATUS parse_config(IN EFI_HANDLE imageHandle, OUT ConfigInfo *cfg);

#endif