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
	if (EFI_ERROR(res)) { goto err_free_none; }

	// get the boot filesystem
	EFI_HANDLE bootPart = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *bootPartFs = NULL;
	res = find_filesystem_for_guid(&bootConfig.bootPartUUID, &bootPart, &bootPartFs);
	if (EFI_ERROR(res)) { goto err_free_cfg; }
	DBG_MSG("Found boot partition\n\r");

	// open the binary to boot
	EFI_FILE *bootRoot = NULL;
	EFI_FILE *bootBinary = NULL;
	res = bootPartFs->OpenVolume(bootPartFs, &bootRoot);
	if (EFI_ERROR(res)) { goto err_free_bootRoot; }

	res = bootRoot->Open(bootRoot, &bootBinary, bootConfig.bootBinPath, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(res)) { goto err_free_bootBin; }
	DBG_MSG("Boot binary file opened\n\r");

	// load the boot binary
	elf_loadinfo_t elfLoadInfo;
	res = elf_load_file(bootBinary, &elfLoadInfo, ELF_LOAD_DISCONTIG);
	if (EFI_ERROR(res)) { goto err_free_loadedBin; }
	DBG_MSG("Boot binary file loaded\n\r");
	DBG_MSG("Boot entry: 0x%lx\n\r", elfLoadInfo.entry);
	elf_print_sections(&elfLoadInfo);

	// map the boot binary to it's expected vaddr
	// create the boot info structure

	// free resources before exiting boot services
	elf_free_loadinfo(&elfLoadInfo);
	bootBinary->Close(bootBinary);
	bootRoot->Close(bootRoot);
	free_config(&bootConfig);

	// exit boot services
	// pass control to the boot binary

	return EFI_SUCCESS;// temporary until control handoff is implemented

	while (1) { __asm__ volatile("hlt"); }// should be unreachable.

	// abnormal exit path - only valid before exiting boot services
err_free_loadedBin:
	elf_free_sections(&elfLoadInfo);
	elf_free_loadinfo(&elfLoadInfo);
err_free_bootBin:
	bootBinary->Close(bootBinary);
err_free_bootRoot:
	bootRoot->Close(bootRoot);
err_free_cfg:
	free_config(&bootConfig);
err_free_none:
	return res;
}