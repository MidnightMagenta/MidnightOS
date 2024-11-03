#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef struct{
	EFI_MEMORY_DESCRIPTOR* mem_map;
	UINTN mem_map_size;
	UINTN mem_map_descriptor_size;
} boot_info_t;

EFI_FILE *LoadFile(EFI_FILE *Directory, CHAR16 *Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_FILE *LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void **) &LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &FileSystem);

	if (Directory == NULL) { FileSystem->OpenVolume(FileSystem, &Directory); }

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS) { return NULL; }
	return LoadedFile;
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

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	//PROLOGUE
	InitializeLib(ImageHandle, SystemTable);
	Print(L"Bootloader started\n\r");
	//!PROLOGUE

	//BODY
	//LOAD KERNEL FILE
	EFI_FILE *Kernel = LoadFile(NULL, L"\\boot\\kernel.elf", ImageHandle, SystemTable);
	if (Kernel == NULL) {
		Print(L"Failed to load kernel \n\r");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Kernel Loaded Successfully \n\r");
	}
	//PARSE ELF HEADERS
	Elf64_Ehdr header;
	UINTN FileInfoSize;
	EFI_FILE_INFO *FileInfo;
	Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
	SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void **) &FileInfo);
	Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void **) &FileInfo);

	UINTN header_size = sizeof(header);
	Kernel->Read(Kernel, &header_size, &header);

	if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC || header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
		Print(L"Failed to verify kernel header\r\n");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Kernel header successfully verified\r\n");
	}

	Elf64_Phdr *phdrs;
	Kernel->SetPosition(Kernel, header.e_phoff);
	UINTN size = header.e_phnum * header.e_phentsize;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void **) &phdrs);
	Kernel->Read(Kernel, &size, phdrs);

	for (Elf64_Phdr *phdr = phdrs; (char *) phdr < (char *) phdrs + header.e_phnum * header.e_phentsize;
		 phdr = (Elf64_Phdr *) ((char *) phdr + header.e_phentsize)) {
		switch (phdr->p_type) {
			case PT_LOAD: {
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void *) segment);
				break;
			}
		}
	}
	Print(L"Kernel successfully loaded\n\r");

	//ACQUIRE NECESSARY BOOT INFORMATION
	boot_info_t boot_info;

	EFI_MEMORY_DESCRIPTOR* map = NULL;
	UINTN map_size, map_key;
	UINTN descriptor_size;
	UINT32 descriptor_version;

	SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);
	SystemTable->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&map);
	SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);

	Print(L"Memory map acquired\n\r");

	boot_info.mem_map = map;
	boot_info.mem_map_size = map_size;
	boot_info.mem_map_descriptor_size = descriptor_size;

	Print(L"Boot info created\n\r");
	//!BODY

	//EPILOGUE
	Print(L"Exiting boot services\n\r");
	SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);

	void (*KernelStart)(boot_info_t*) = ((__attribute__((sysv_abi)) void (*)(boot_info_t*)) header.e_entry);
	KernelStart(&boot_info);

	Print(L"Returned from kernel\n\r");

	return EFI_SUCCESS;
	//!EPILOGUE
}