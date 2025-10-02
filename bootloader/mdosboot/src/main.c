#include "../include/cfgparse.h"
#include "../include/debug.h"
#include <efi.h>
#include <efilib.h>

// TODO: uncomment this
// #ifndef HAVE_USE_MS_ABI
// #error "Compiler must support using MS ABI"
// #endif

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS res = EFI_SUCCESS;
	InitializeLib(imageHandle, systemTable);

	ConfigInfo info;
	res = parse_config(imageHandle, &info);
	if (EFI_ERROR(res)) { DBG_MSG("Failed to parse boot configuration with: 0x%lx\n\r", res); }

	UINT16 *uuid;
	gBS->AllocatePool(EfiLoaderData, 32, (void **) &uuid);
	GuidToString(uuid, &info.bootPartUUID);
	DBG_MSG("Boot configuration\n\r\tDisk GUID: %s\n\r\tBoot path: %s\n\r", uuid, info.bootBinPath);

	// free resources before exiting boot services
	gBS->FreePool(info.bootBinPath);
	return res;
}