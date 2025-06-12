#include "../include/basics.h"

EFI_STATUS open_file(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable,
					 EFI_FILE **file) {
	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem;
	systemTable->BootServices->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &loadedImage);
	systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid,
											  (void **) &fileSystem);

	if (!directory) { fileSystem->OpenVolume(fileSystem, &directory); }
	return directory->Open(directory, file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
}

int mem_compare(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = (unsigned char *) aptr, *b = (unsigned char *) bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}