#include <efi.h>
#include <efilib.h>
#include <elf.h>

#define VERBOSE_REPORTING 1

#define HandleError(fmt, status)                                                                                                                               \
	if (status != EFI_SUCCESS) {                                                                                                                               \
		Print(fmt, status);                                                                                                                                    \
		return status;                                                                                                                                         \
	}
#define ALIGN_ADDR(val, alignment, castType) ((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))

typedef struct {
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN mapSize;
	UINTN mapDescriptorSize;
	uint64_t *bootstrapPML4;
} bootInfot_t;

EFI_FILE *OpenFile(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, EFI_STATUS *status) {
	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem;
	systemTable->BootServices->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &loadedImage);
	systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &fileSystem);

	if (!directory) { fileSystem->OpenVolume(fileSystem, &directory); }

	EFI_FILE *file;
	*status = directory->Open(directory, &file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	return file;
}

int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;

	InitializeLib(imageHandle, systemTable);
#if VERBOSE_REPORTING
	Print(L"Bootloader started\n\r");
#endif
	//Open the bootloader_l3.elf file
	EFI_FILE *bootloader_l3 = OpenFile(NULL, L"\\boot\\bootloaderl3.elf", imageHandle, systemTable, &status);
	//if the file hasn't been opened successfully, print an error.
	if (status != EFI_SUCCESS) {
		Print(L"Could not open bootloader_l3.elf: ");
		switch (status) {
			case EFI_NOT_FOUND:
				Print(L"File not found. Code: 0x%lx\n\r", status);
				break;
			case EFI_NO_MEDIA:
				Print(L"The device has no medium. Code: 0x%lx\n\r", status);
				break;
			case EFI_MEDIA_CHANGED:
				Print(L"The device has a different medium or the medium is no longer supported. Code: 0x%lx\n\r", status);
				break;
			case EFI_DEVICE_ERROR:
				Print(L"The device has reported an error. Code: 0x%lx\n\r", status);
				break;
			case EFI_VOLUME_CORRUPTED:
				Print(L"The file system strctures are corrupted. Code: 0x%lx\n\r", status);
				break;
			case EFI_ACCESS_DENIED:
				Print(L"Access denied. Code: 0x%lx\n\r", status);
				break;
			case EFI_OUT_OF_RESOURCES:
				Print(L"Not enough resources to open the file. Code: 0x%lx\n\r", status);
				break;
			default:
				Print(L"Unknown error. Code: 0x%lx\n\r", status);
				break;
		}

		return status;
	} else if (!bootloader_l3) {
		Print(L"Failed to open bootloader_l3.elf\n\r");
		return EFI_LOAD_ERROR;
	}
#if VERBOSE_REPORTING
	else {
		Print(L"bootloader_l3.elf opened successfully\n\r");
	}
#endif

	UINTN fileInfoSize = 0;
	EFI_FILE_INFO *fileInfo;
	status = bootloader_l3->GetInfo(bootloader_l3, &gEfiFileInfoGuid, &fileInfoSize, NULL);
	if (status != EFI_BUFFER_TOO_SMALL) {
		Print(L"Failed to get file info size: 0x%lx", status);
		return status;
	}
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, fileInfoSize, (void **) &fileInfo);
	HandleError(L"Failed to allocate memory pool: 0x%lx", status);
	status = bootloader_l3->GetInfo(bootloader_l3, &gEfiFileInfoGuid, &fileInfoSize, (void **) &fileInfo);
	HandleError(L"Failed to get file info: 0x%lx", status);

	Elf64_Ehdr header;
	UINTN headerSize = sizeof(header);
	bootloader_l3->Read(bootloader_l3, &headerSize, &header);

	if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) {
		Print(L"Invalid bootloader_l3 binary elf magic\n\r");
		return EFI_LOAD_ERROR;
	}
	if (header.e_ident[EI_CLASS] != ELFCLASS64) {
		Print(L"bootloader_l3 binary is not 64 bit format\n\r");
		return EFI_LOAD_ERROR;
	}
	if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
		Print(L"bootloader_l3 binary has incorrect endianness\n\r");
		return EFI_LOAD_ERROR;
	}
	if (header.e_type != ET_EXEC) {
		Print(L"bootloader_l3 binary is not executable\n\r");
		return EFI_LOAD_ERROR;
	}
	if (header.e_machine != EM_X86_64) {
		Print(L"bootloader_l3 binary not x86_64 architecture\n\r");
		return EFI_LOAD_ERROR;
	}
	if (header.e_version != EV_CURRENT) {
		Print(L"Invalid kernal binary elf version\n\r");
		return EFI_LOAD_ERROR;
	}

#if VERBOSE_REPORTING
	Print(L"bootloader_l3 binary elf header successfully verified\n\r");
#endif

	bootloader_l3->SetPosition(bootloader_l3, header.e_phoff);
	Elf64_Phdr *phdrs = NULL;
	UINTN size = header.e_phnum * header.e_phentsize;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, size, (void **) &phdrs);
	HandleError(L"Failed to allocate memory pool for p headers: 0x%lx", status);
	status = bootloader_l3->Read(bootloader_l3, &size, phdrs);
	HandleError(L"Failed to read p headers: 0x%lx", status);
	if (!phdrs) {
		Print(L"Failed to read p headers: phdrs == nullptr\n\r");
		return EFI_LOAD_ERROR;
	}

#if VERBOSE_REPORTING
	Print(L"bootloader_l3 p headers successfully loaded\n\r");
#endif

	UINTN totalSegmentsSize = 0;
	for (Elf64_Phdr *phdr = phdrs; (char *) phdr < (char *) phdrs + header.e_phnum * header.e_phentsize;
		 phdr = (Elf64_Phdr *) ((char *) phdr + header.e_phentsize)) {
		if (phdr->p_type == PT_LOAD) { totalSegmentsSize += ALIGN_ADDR(phdr->p_filesz, phdr->p_align, UINTN); }
	}
#if VERBOSE_REPORTING
	Print(L"Total size to load: 0x%x\n\r", totalSegmentsSize);
#endif

	EFI_MEMORY_DESCRIPTOR *map = NULL;
	UINTN mapSize = 0, mapKey, descriptorSize;
	UINT32 descriptorVersion;

	status = systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
	mapSize += descriptorSize * 5;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void **) &map);
	HandleError(L"Failed to allocate memory pool for map: 0x%lx\n\r", status);
	status = systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
	HandleError(L"Failed to obtain memory map: 0x%lx\n\r", status);

	Elf64_Addr beginAddress = 0;
	Elf64_Addr endAddress = 0;

	UINT8 memoryFound = 0;
	for (EFI_MEMORY_DESCRIPTOR *descriptor = map; descriptor < (EFI_MEMORY_DESCRIPTOR *) ((UINTN) map + mapSize);
		 descriptor = (EFI_MEMORY_DESCRIPTOR *) ((UINTN) descriptor + descriptorSize)) {
		if (descriptor->Type == EfiConventionalMemory && descriptor->NumberOfPages > (totalSegmentsSize / 0x1000)) {
			beginAddress = (Elf64_Addr) descriptor->PhysicalStart;
			memoryFound = 1;
			break;
		}
	}

	if (!memoryFound) {
		Print(L"Failed to find suitable memory\n\r");
		return EFI_LOAD_ERROR;
	}

#if VERBOSE_REPORTING
	Print(L"Found suitable memory\n\r");
#endif

	endAddress = beginAddress;
	for (Elf64_Phdr *phdr = phdrs; (char *) phdr < (char *) phdrs + header.e_phnum * header.e_phentsize;
		 phdr = (Elf64_Phdr *) ((char *) phdr + header.e_phentsize)) {
		if (phdr->p_type == PT_LOAD) {
			int pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			systemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pageCount, &endAddress);

			status = bootloader_l3->SetPosition(bootloader_l3, phdr->p_offset);
			HandleError(L"Failed to set position in bootloader_l3.elf file protocol: 0x%lx\n\r", status);
			UINTN size = phdr->p_filesz;
			status = bootloader_l3->Read(bootloader_l3, &size, (void *) endAddress);
			HandleError(L"Failed to read segment: 0x%lx\n\r", status);
			endAddress += pageCount * 0x1000;
		}
	}

#if VERBOSE_REPORTING
	Print(L"bootloader_l3 segments loaded successfully\n\r");
#endif

	bootloader_l3->Close(bootloader_l3);

	void (*_start)(bootInfot_t *) = ((__attribute__((sysv_abi)) void (*)(bootInfot_t *)) header.e_entry);

#if VERBOSE_REPORTING
	Print(L"Found entry symbol at: 0x%lx\n\r", _start);
#endif

	systemTable->BootServices->FreePool(map);
	map = NULL;
	mapSize = 0;

	status = systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
	mapSize += descriptorSize * 5;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void **) &map);
	HandleError(L"Failed to allocate memory pool for map: 0x%lx\n\r", status);
	status = systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
	HandleError(L"Failed to obtain memory map: 0x%lx\n\r", status);

	return EFI_SUCCESS;
}