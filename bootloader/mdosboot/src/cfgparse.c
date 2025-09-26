#include "../include/cfgparse.h"
#include "../include/debug.h"
#include "../include/fs.h"

static EFI_STATUS open_config_file(EFI_FILE **cfg, EFI_HANDLE imageHandle) {
	EFI_STATUS res;

	EFI_LOADED_IMAGE_PROTOCOL *imageProtocol = NULL;
	res = gBS->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &imageProtocol);
	if (EFI_ERROR(res) || imageProtocol == NULL) {
		DBG_MSG("[%a %d] Failed to obtain loaded image protocol with: %lx\n\r", __func__, __LINE__, res);
		return res;
	}

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *espFS = NULL;
	res = gBS->HandleProtocol(imageProtocol->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &espFS);
	if (EFI_ERROR(res) || espFS == NULL) {
		DBG_MSG("[%a %d] Failed to obtain loaded image device filesystem protocol with: %lx\n\r", __func__, __LINE__,
				res);
		return res;
	}

	EFI_FILE *root = NULL;
	res = espFS->OpenVolume(espFS, &root);
	if (EFI_ERROR(res) || root == NULL) {
		DBG_MSG("[%a %d] Failed to open root directory with: %lx\n\r", __func__, __LINE__, res);
		return res;
	}

	res = root->Open(root, cfg, L"BOOT\\MDOSBOOT.CFG", EFI_FILE_READ_ONLY, 0);
	if (EFI_ERROR(res) || *cfg == NULL) {
		DBG_MSG("[%a %d] Failed to open configuration file with: %lx\n\r", __func__, __LINE__, res);
		root->Close(root);
		return res;
	}

	root->Close(root);
	return EFI_SUCCESS;
}

// static EFI_STATUS read_config(CHAR16 *cfgBuffer, UINTN cfgSize) {
// 	UINTN i = 0;
// 	while (i < cfgSize) {}
// }

EFI_STATUS parse_config(IN EFI_HANDLE imageHandle, OUT ConfigInfo *cfg) {
	if (imageHandle == NULL || cfg == NULL) { return EFI_INVALID_PARAMETER; }
	EFI_STATUS res = EFI_SUCCESS;

	EFI_FILE *cfgFile = NULL;
	CHAR16 *cfgBuffer = NULL;

	res = open_config_file(&cfgFile, imageHandle);
	if (cfgFile == NULL) { goto cleanup; }
	if (EFI_ERROR(res)) { goto cleanup; }

	UINTN fileSize = 0;
	res = get_file_size(cfgFile, &fileSize);
	if (EFI_ERROR(res) || fileSize == 0) { goto cleanup; }
	// DBG_MSG("Config file size: %d\n\r", fileSize);

	res = gBS->AllocatePool(EfiLoaderData, fileSize, (void **) &cfgBuffer);
	if (EFI_ERROR(res)) { goto cleanup; }
	ZeroMem(cfgBuffer, fileSize);

	res = cfgFile->Read(cfgFile, &fileSize, cfgBuffer);
	if (EFI_ERROR(res)) { goto cleanup; }

cleanup:
	if (cfgBuffer != NULL) { gBS->FreePool(cfgBuffer); }
	if (cfgFile != NULL) { cfgFile->Close(cfgFile); }
	return res;
}
