#include <efi.h>
#include <efilib.h>

// load necessary drivers and call mdosboot.efi

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;
	InitializeLib(imageHandle, systemTable);

	EFI_HANDLE *handles;
	UINTN handleCount;

	status = BS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handleCount, &handles);
	if (EFI_ERROR(status)) { return status; }

	for (UINTN i = 0; i < handleCount; i++) {
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
		EFI_FILE_PROTOCOL *root;
		EFI_FILE_PROTOCOL *file;

		status = BS->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (void **) &fs);
		if (EFI_ERROR(status)) { continue; }

		status = fs->OpenVolume(fs, &root);
		if (EFI_ERROR(status)) { continue; }

		status = root->Open(root, &file, L"\\BOOT\\MDOSBOOT.EFI", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
		if (EFI_ERROR(status)) { continue; }

		EFI_HANDLE newImageHandle;
		EFI_DEVICE_PATH_PROTOCOL *path = FileDevicePath(handles[i], L"\\BOOT\\MDOSBOOT.EFI");

		status = BS->LoadImage(FALSE, imageHandle, path, NULL, 0, &newImageHandle);
		if (EFI_ERROR(status)) { return status; }

		status = BS->StartImage(newImageHandle, NULL, NULL);
		if (EFI_ERROR(status)) { return status; }

		BS->UnloadImage(newImageHandle);
		BS->FreePool(handles);
		return EFI_SUCCESS;
	}

	return EFI_LOAD_ERROR;
}