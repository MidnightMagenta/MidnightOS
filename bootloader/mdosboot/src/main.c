#include "../include/cfgparse.h"
#include "../include/debug.h"
#include "../include/elf.h"
#include "../include/fs.h"
#include <efi.h>
#include <efilib.h>

#ifndef HAVE_USE_MS_ABI
#error "Compiler must support using MS ABI"
#endif

// FIXME: free resources on error exit
EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS res = EFI_SUCCESS;
	InitializeLib(imageHandle, systemTable);

	// get boot configuration
	configinfo_t bootConfig;
	res = parse_config(imageHandle, &bootConfig);
	if (EFI_ERROR(res)) { DBG_MSG("Failed to parse boot configuration with: 0x%lx\n\r", res); }

	// get the boot filesystem
	EFI_HANDLE bootPart = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *bootPartFs = NULL;
	res = find_filesystem_for_guid(&bootConfig.bootPartUUID, &bootPart, &bootPartFs);
	if (EFI_ERROR(res)) {
		gBS->FreePool(bootConfig.bootBinPath);
		DBG_WARN("Failed to find boot partition with: 0x%lx\n\r", res);
		return EFI_NOT_FOUND;
	}
	DBG_MSG("Found boot partition\n\r");

	// open the binary to boot
	EFI_FILE *bootRoot = NULL;
	EFI_FILE *bootBinary = NULL;
	res = bootPartFs->OpenVolume(bootPartFs, &bootRoot);
	if (EFI_ERROR(res)) {
		DBG_MSG("Failed to open boot partition root volume with: 0x%lx\n\r", res);
		return res;
	}

	res = bootRoot->Open(bootRoot, &bootBinary, bootConfig.bootBinPath, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(res)) {
		DBG_MSG("Failed to open boot binary file with: 0x%lx\n\r", res);
		return res;
	}
	DBG_MSG("Boot binary file opened\n\r");

	// load the boot binary
	elf_loadinfo_t elfInfo;
	res = elf_load_file(bootBinary, &elfInfo);
	if (EFI_ERROR(res)) { return res; }
	DBG_MSG("Boot binary file loaded\n\r");

	// map the boot binary to it's expected vaddr
	// create the boot info structure

	// free resources before exiting boot services
	bootBinary->Close(bootBinary);
	bootRoot->Close(bootRoot);
	free_config(&bootConfig);

	// exit boot services
	// pass control to the boot binary

	return res;
}