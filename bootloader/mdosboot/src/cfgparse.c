#include "../include/cfgparse.h"
#include "../include/debug.h"
#include "../include/fs.h"

static EFI_STATUS open_config_file(EFI_FILE **cfg, EFI_HANDLE imageHandle) {
	EFI_STATUS res;

	EFI_LOADED_IMAGE_PROTOCOL *imageProtocol = NULL;
	res = gBS->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &imageProtocol);
	if (EFI_ERROR(res) || imageProtocol == NULL) {
		DBG_WARN("[%a %d] Failed to obtain loaded image protocol with: %lx\n\r", __func__, __LINE__, res);
		return res;
	}

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *espFS = NULL;
	res = gBS->HandleProtocol(imageProtocol->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &espFS);
	if (EFI_ERROR(res) || espFS == NULL) {
		DBG_WARN("[%a %d] Failed to obtain loaded image device filesystem protocol with: %lx\n\r", __func__, __LINE__,
				 res);
		return res;
	}

	EFI_FILE *root = NULL;
	res = espFS->OpenVolume(espFS, &root);
	if (EFI_ERROR(res) || root == NULL) {
		DBG_WARN("[%a %d] Failed to open root directory with: %lx\n\r", __func__, __LINE__, res);
		return res;
	}

	res = root->Open(root, cfg, L"BOOT\\BOOT.CFG", EFI_FILE_READ_ONLY, 0);
	if (EFI_ERROR(res) || *cfg == NULL) {
		DBG_WARN("[%a %d] Failed to open configuration file with: %lx\n\r", __func__, __LINE__, res);
		root->Close(root);
		return res;
	}

	root->Close(root);
	return EFI_SUCCESS;
}

static EFI_STATUS verify_header(const MDBC_Header *header, char *cfgBuffer, UINTN cfgSize) {
	if (CompareMem(header->magic, MDBC_Magic, 4) != 0) { return EFI_UNSUPPORTED; }
	if (header->version != 1) { return EFI_INCOMPATIBLE_VERSION; }

	crc32_t cfgCrc32 = crc32((cfgBuffer + header->dataOffset), cfgSize - header->dataOffset, 0xFFFFFFFF);
	if (cfgCrc32 != header->crc32) { return EFI_CRC_ERROR; }

	return EFI_SUCCESS;
}

static EFI_STATUS read_config(char *cfgBuffer, UINTN cfgSize, ConfigInfo *cfg) {
	EFI_STATUS res;

	MDBC_Header header;
	CopyMem((void *) &header, cfgBuffer, sizeof(MDBC_Header));

	res = verify_header(&header, cfgBuffer, cfgSize);
	if (EFI_ERROR(res)) {
		DBG_WARN("Failed to verify boot configuration file header with %lx\n\r", res);
		return res;
	}

	UINT8 *ptr = cfgBuffer + header.dataOffset;
	UINT8 *end = cfgBuffer + cfgSize;

	while (ptr < end) {
		CHAR8 *name = (CHAR8 *) ptr;
		UINTN nameLen = strlena(name);
		ptr += nameLen + 1;

		if (ptr + sizeof(UINT16) > end) { return EFI_COMPROMISED_DATA; }

		UINT16 size = *(UINT16 *) ptr;
		ptr += sizeof(UINT16);

		if (ptr + size + 2 > end) { return EFI_COMPROMISED_DATA; }

		VOID *data = ptr;
		ptr += size;

		if (ptr[0] != 0x00 || ptr[1] != 0x0A) { return EFI_COMPROMISED_DATA; }
		ptr += 2;

		if (strcmpa(name, "BOOT_DISK") == 0) {
			CopyMem(&cfg->bootPartUUID, data, sizeof(EFI_GUID));
		} else if (strcmpa(name, "BOOT_PATH") == 0) {
			res = gBS->AllocatePool(EfiLoaderData, size, (void **) &cfg->bootBinPath);
			if (EFI_ERROR(res)) { return res; }
			CopyMem(cfg->bootBinPath, data, size);
		} else {
			return EFI_INVALID_PARAMETER;
		}
	}

	return EFI_SUCCESS;
}

EFI_STATUS parse_config(IN EFI_HANDLE imageHandle, OUT ConfigInfo *cfg) {
	if (imageHandle == NULL || cfg == NULL) { return EFI_INVALID_PARAMETER; }
	EFI_STATUS res = EFI_SUCCESS;

	UINTN fileSize = 0;
	EFI_FILE *cfgFile = NULL;
	char *cfgBuffer = NULL;

	res = open_config_file(&cfgFile, imageHandle);
	if (EFI_ERROR(res)) { return res; }

	res = get_file_size(cfgFile, &fileSize);
	if (EFI_ERROR(res) || fileSize == 0) {
		cfgFile->Close(cfgFile);
		return res;
	}
	DBG_MSG("Config file size: %d\n\r", fileSize);

	res = gBS->AllocatePool(EfiLoaderData, fileSize, (void **) &cfgBuffer);
	if (EFI_ERROR(res)) {
		cfgFile->Close(cfgFile);
		return res;
	}
	ZeroMem(cfgBuffer, fileSize);

	res = cfgFile->Read(cfgFile, &fileSize, cfgBuffer);
	if (EFI_ERROR(res)) {
		gBS->FreePool(cfgBuffer);
		cfgFile->Close(cfgFile);
		return res;
	}

	res = read_config(cfgBuffer, fileSize, cfg);
	if (EFI_ERROR(res)) {
		gBS->FreePool(cfgBuffer);
		cfgFile->Close(cfgFile);
		return res;
	}

	cfgFile->Close(cfgFile);
	gBS->FreePool(cfgBuffer);
	return EFI_SUCCESS;
}
