#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>

typedef struct {
	void *base_addr;
	size_t size;
	unsigned int width;
	unsigned int height;
	unsigned int pixel_per_scanline;
} FrameBuffer_t;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char char_size;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER *header;
	void *glyphs;
} PSF1_FONT;

typedef struct {
	FrameBuffer_t *frameBuffer;
	PSF1_FONT *initialFont;
} BootInfo_t;

FrameBuffer_t frameBuffer;
FrameBuffer_t *InitializeGOP() {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL,
							   (void **) &gop);
	if (EFI_ERROR(status)) {
		Print(L"Unable to locate GOP\n\r");
		return NULL;
	} else {
		Print(L"GOP located\n\r");
	}

	frameBuffer.base_addr = (void *) gop->Mode->FrameBufferBase;
	frameBuffer.size = gop->Mode->FrameBufferSize;
	frameBuffer.width = gop->Mode->Info->HorizontalResolution;
	frameBuffer.height = gop->Mode->Info->VerticalResolution;
	frameBuffer.pixel_per_scanline = gop->Mode->Info->PixelsPerScanLine;

	return &frameBuffer;
}

EFI_FILE *LoadFile(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle,
				   EFI_SYSTEM_TABLE *systemTable) {
	EFI_FILE *loadedFile;

	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
	systemTable->BootServices->HandleProtocol(
			imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &loadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem;
	systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle,
											  &gEfiSimpleFileSystemProtocolGuid,
											  (void **) &fileSystem);

	if (directory == NULL) { fileSystem->OpenVolume(fileSystem, &directory); }

	EFI_STATUS s = directory->Open(directory, &loadedFile, path,
								   EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if (s != EFI_SUCCESS) { return NULL; }
	return loadedFile;
}

PSF1_FONT *LoadInitialPSF1Font(EFI_FILE *directory, CHAR16 *path,
							   EFI_HANDLE imageHandle,
							   EFI_SYSTEM_TABLE *systemTable) {
	EFI_FILE *font = LoadFile(directory, path, imageHandle, systemTable);
	if (font == NULL) { return NULL; }
	PSF1_HEADER *fontHeader;
	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER),
											(void **) &fontHeader);

	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	if (fontHeader->magic[0] != PSF1_MAGIC0 ||
		fontHeader->magic[1] != PSF1_MAGIC1) {
		return NULL;
	}

	UINTN glyphBufferSize = fontHeader->char_size * 256;
	if (fontHeader->mode == 1) {
		glyphBufferSize = fontHeader->char_size * 512;
	}

	void *glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		systemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize,
												(void **) &glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT *finishedFont;
	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT),
											(void **) &finishedFont);

	finishedFont->header = fontHeader;
	finishedFont->glyphs = glyphBuffer;
	return finishedFont;
}

int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr;
	const unsigned char *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) {
			return -1;
		} else if (a[i] > b[i]) {
			return 1;
		}
	}
	return 0;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	InitializeLib(imageHandle, systemTable);
	Print(L"String blah blah blah \n\r");

	EFI_FILE *Kernel = LoadFile(NULL, L"kernel.elf", imageHandle, systemTable);
	if (Kernel == NULL) {
		Print(L"Could not load kernel \n\r");
	} else {
		Print(L"Kernel Loaded Successfully \n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO *FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		systemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize,
												(void **) &FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize,
						(void **) &FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
		Print(L"kernel format is bad\r\n");
	} else {
		Print(L"kernel header successfully verified\r\n");
	}

	Elf64_Phdr *phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		systemTable->BootServices->AllocatePool(EfiLoaderData, size,
												(void **) &phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for (Elf64_Phdr *phdr = phdrs;
		 (char *) phdr < (char *) phdrs + header.e_phnum * header.e_phentsize;
		 phdr = (Elf64_Phdr *) ((char *) phdr + header.e_phentsize)) {
		switch (phdr->p_type) {
			case PT_LOAD: {
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				systemTable->BootServices->AllocatePages(
						AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void *) segment);
				break;
			}
		}
	}


	Print(L"Kernel loaded\n\r");

	PSF1_FONT *initialFont = LoadInitialPSF1Font(NULL, L"zap-light18.psf",
												 imageHandle, systemTable);
	if (initialFont == NULL) {
		Print(L"Could not load initial font\n\r");
	} else {
		Print(L"Initial font loaded successfully. Char size: %d\n\r",
			  initialFont->header->char_size);
	}

	void (*kernelStart)(BootInfo_t *) =
			((__attribute__((sysv_abi)) void (*)(BootInfo_t *)) header.e_entry);

	FrameBuffer_t *fb = InitializeGOP();

	Print(L"Framebuffer addr: 0x%x\n\r", fb->base_addr);
	Print(L"Framebuffer size: 0x%x\n\r", fb->size);
	Print(L"Framebuffer width: %d\n\r", fb->width);
	Print(L"Framebuffer height: %d\n\r", fb->height);
	Print(L"Framebuffer pps: %d\n\r", fb->pixel_per_scanline);

	BootInfo_t bootInfo;
	bootInfo.frameBuffer = fb;
	bootInfo.initialFont = initialFont;

	kernelStart(&bootInfo);

	return EFI_SUCCESS;// Exit the UEFI application
}
