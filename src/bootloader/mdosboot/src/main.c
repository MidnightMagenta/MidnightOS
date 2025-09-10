#include <efi.h>
#include <efilib.h>
#include "../include/debug.h"
#include "../include/cfgparse.h"

#ifndef HAVE_USE_MS_ABI
#error "Compiler must support using MS ABI"
#endif

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS res = EFI_SUCCESS;
	InitializeLib(imageHandle, systemTable);

	Print(L"Hello bootloader\n\r");
	ConfigInfo info;
	parse_config(imageHandle, &info);
	return EFI_SUCCESS;
}