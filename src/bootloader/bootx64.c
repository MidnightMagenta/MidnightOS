#include <efi.h>
#include <efilib.h>
#include <elf.h>

#define VERBOSE_REPORTING 1
#define BOOTSTRAP_HEAP_PAGE_COUNT 12207//approx. 50 mb

#define HandleError(fmt, status)                                                                                       \
	if (status != EFI_SUCCESS) {                                                                                       \
		Print(L"Critical error: " fmt L": 0x%lx\n\r", status);                                                         \
		return status;                                                                                                 \
	}
#define ALIGN_ADDR(val, alignment, castType)                                                                           \
	((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))

#define PT_ENTRY(addr) (addr >> 12) & 0x1FF
#define PD_ENTRY(addr) (addr >> 21) & 0x1FF
#define PDPT_ENTRY(addr) (addr >> 30) & 0x1FF
#define PML4_ENTRY(addr) (addr >> 39) & 0x1FF

#define PAGE_PRESENT 1 << 0
#define PAGE_WRITABLE 1 << 1
#define PAGE_SUPERVISOR 1 << 2
#define PAGE_WSP PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864ab572

typedef struct {
	uint16_t magic;
	uint8_t fontMode;
	uint8_t charSize;
} PSF1_Header;

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t glyphCount;
	uint32_t bytesPerGlyph;
	uint32_t height;
	uint32_t width;
} PSF2_Header;

typedef struct {
	PSF1_Header *header;
	void *glyphs;
} PSF1_Font;

typedef struct {
	Elf64_Addr paddr;
	Elf64_Addr vaddr;
	UINTN pageCount;
	Elf64_Word flags;
} LoadedSectionInfo;

typedef struct {
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN size;
	UINTN key;
	UINTN descriptorSize;
	UINT32 descriptorVersion;
} MemMap;

typedef struct {
	void *bufferBase;
	UINTN bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanline;
} GOPFramebuffer;

typedef struct {
	PSF1_Font *basicFont;
	GOPFramebuffer *framebuffer;
} BootExtra;

typedef struct {
	void *baseAddr;
	void *topAddr;
	void *basePaddr;
	void *topPaddr;
	size_t size;
} BootstrapMemoryRegion;

typedef struct {
	MemMap *map;
	uint64_t *pml4;
	BootExtra bootExtra;
	BootstrapMemoryRegion bootstrapMem;
} BootInfo;

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
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}

int verify_ehdr(Elf64_Ehdr header) {
	if (mem_compare(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) {
		Print(L"Invalid kernel binary elf magic\n\r");
		return 0;
	}
	if (header.e_ident[EI_CLASS] != ELFCLASS64) {
		Print(L"kernel binary is not 64 bit format\n\r");
		return 0;
	}
	if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
		Print(L"kernel binary has incorrect endianness\n\r");
		return 0;
	}
	if (header.e_type != ET_EXEC) {
		Print(L"kernel binary is not executable\n\r");
		return 0;
	}
	if (header.e_machine != EM_X86_64) {
		Print(L"kernel binary not x86_64 architecture\n\r");
		return 0;
	}
	if (header.e_version != EV_CURRENT) {
		Print(L"Invalid kernal binary elf version\n\r");
		return 0;
	}
	return 1;
}

EFI_STATUS get_phdrs(EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr ehdr, EFI_FILE *elfFile, Elf64_Phdr **phdrs) {
	EFI_STATUS status;
	UINTN phdrBuffSize = ehdr.e_phnum * ehdr.e_phentsize;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, phdrBuffSize, (void **) phdrs);
	HandleError(L"Failed to allocate phdr buffer", status);

	status = elfFile->Read(elfFile, &phdrBuffSize, *phdrs);
	HandleError(L"Failed to read phdrs", status);
	return EFI_SUCCESS;
}

UINTN count_loadable_segments(Elf64_Phdr *phdrs, Elf64_Half phnum, Elf64_Half phentsize) {
	UINTN phdrCount = 0;
	for (Elf64_Phdr *phdr = phdrs; (char *) phdr < (char *) phdrs + phnum * phentsize;
		 phdr = (Elf64_Phdr *) ((char *) phdr + phentsize)) {
		if (phdr->p_type == PT_LOAD) { phdrCount++; }
	}
	return phdrCount;
}

EFI_STATUS load_kernel(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr *pEhdr,
					   UINTN *sectionInfoCount, LoadedSectionInfo **sectionInfos) {
	EFI_STATUS status;
	EFI_FILE *kernel = NULL;
	status = open_file(NULL, L"\\boot\\kernel.elf", imageHandle, systemTable, &kernel);
	HandleError(L"Failed to open kernel file: 0x%lx\n\r", status);

	if (!kernel) {
		Print(L"Kernel is nullptr\n\r");
		return EFI_LOAD_ERROR;
	}

#if VERBOSE_REPORTING
	Print(L"Kernel image located...\n\r");
#endif

	Elf64_Ehdr ehdr;
	UINTN ehdrSize = sizeof(ehdr);
	kernel->Read(kernel, &ehdrSize, &ehdr);
	if (!verify_ehdr(ehdr)) { return EFI_LOAD_ERROR; }
	*pEhdr = ehdr;

#if VERBOSE_REPORTING
	Print(L"Kernel image ehdr valid...\n\r");
#endif

	Elf64_Phdr *phdrs = NULL;
	status = get_phdrs(systemTable, ehdr, kernel, &phdrs);
	if (status != EFI_SUCCESS) { return status; }
	UINTN phdrCount = count_loadable_segments(phdrs, ehdr.e_phnum, ehdr.e_phentsize);
	*sectionInfoCount = phdrCount;

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, phdrCount * sizeof(LoadedSectionInfo),
													 (void **) sectionInfos);
	if (status != EFI_SUCCESS) { return status; }

	{
		Elf64_Phdr *phdr = phdrs;
		LoadedSectionInfo *sectionInfo = *sectionInfos;

		while ((char *) phdr < (char *) phdrs + ehdr.e_phnum * ehdr.e_phentsize &&
			   (char *) sectionInfo < (char *) *sectionInfos + phdrCount * sizeof(LoadedSectionInfo)) {
			if (phdr->p_type == PT_LOAD) {
				int pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pageCount,
																  &sectionInfo->paddr);
				if (status != EFI_SUCCESS) { return status; }
				ZeroMem((void *) sectionInfo->paddr, pageCount * 0x1000);

				status = kernel->SetPosition(kernel, phdr->p_offset);
				if (status != EFI_SUCCESS) { return status; }
				UINTN size = phdr->p_memsz;
				status = kernel->Read(kernel, &size, (void *) sectionInfo->paddr);

				if (status != EFI_SUCCESS) { return status; }

				sectionInfo->vaddr = phdr->p_vaddr;
				sectionInfo->pageCount = pageCount;
				sectionInfo->flags = phdr->p_flags;
#if VERBOSE_REPORTING
				Print(L"Loading section...\n\r   Vaddr: 0x%lx\n\r   Paddr: 0x%lx\n\r   Page count: %u\n\r",
					  sectionInfo->vaddr, sectionInfo->paddr, sectionInfo->pageCount);
#endif
				sectionInfo = (LoadedSectionInfo *) ((char *) sectionInfo + sizeof(LoadedSectionInfo));
			}
			phdr = (Elf64_Phdr *) ((char *) phdr + ehdr.e_phentsize);
		}
	}

	kernel->Close(kernel);

#if VERBOSE_REPORTING
	Print(L"Kernel image loaded successfuly...\n\r");
#endif

	return EFI_SUCCESS;
}

EFI_STATUS init_GOP(EFI_SYSTEM_TABLE *systemTable, GOPFramebuffer *framebuffer) {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	status = systemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);
	HandleError(L"Failed to locate GOP", status);

	framebuffer->bufferBase = (void *) gop->Mode->FrameBufferBase;
	framebuffer->bufferSize = gop->Mode->FrameBufferSize;
	framebuffer->width = gop->Mode->Info->HorizontalResolution;
	framebuffer->height = gop->Mode->Info->VerticalResolution;
	framebuffer->pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

#if VERBOSE_REPORTING
	Print(L"GOP initialized...\n\r");
#endif

	return EFI_SUCCESS;
}

EFI_STATUS get_PSF1_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path, PSF1_Font *font) {
	EFI_FILE *fontFile = NULL;
	EFI_STATUS status = open_file(NULL, path, imageHandle, systemTable, &fontFile);
	HandleError(L"Failed to open font file", status);

	UINTN bytesToRead = 4;
	uint32_t PSF_Magic;
	fontFile->Read(fontFile, &bytesToRead, &PSF_Magic);

	if ((PSF_Magic & 0xFFFF) != PSF1_MAGIC) {
		Print(L"Failed to verify font magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	}

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_Header), (void **) &font->header);
	HandleError(L"Failed to allocate font header memory", status);
	fontFile->SetPosition(fontFile, 0);
	bytesToRead = sizeof(PSF1_Header);
	fontFile->Read(fontFile, &bytesToRead, font->header);

	UINTN glyphBufferSize = font->header->charSize * 256;
	if (font->header->fontMode == 1) { glyphBufferSize = font->header->charSize * 512; }

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void **) &font->glyphs);
	HandleError(L"Failed to allocate font buffer memory", status);
	fontFile->SetPosition(fontFile, sizeof(PSF1_Header));
	fontFile->Read(fontFile, &glyphBufferSize, font->glyphs);

	fontFile->Close(fontFile);
	return EFI_SUCCESS;
}

EFI_STATUS get_PSF2_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path) {
	EFI_FILE *fontFile = NULL;
	EFI_STATUS status = open_file(NULL, path, imageHandle, systemTable, &fontFile);
	HandleError(L"Failed to open font file", status);

	UINTN bytesToRead = 4;
	uint32_t PSF_Magic;
	fontFile->Read(fontFile, &bytesToRead, &PSF_Magic);

	if (PSF_Magic == PSF2_MAGIC) {
		//read PSF2 font
		//TODO: implement PSF2 font reading
		Print(L"PSF2 fonts not supported. Magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Failed to verify font magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	}
}

EFI_STATUS map_page_identity(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_PHYSICAL_ADDRESS addr) {
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS newPage;
	uint64_t *pdpt = NULL;
	uint64_t *pd = NULL;
	uint64_t *pt = NULL;

#if VERBOSE_REPORTING
	Print(L"Mapping page identity...\n\r   addr: 0x%lx\n\r", addr);
#endif

	if (!(pml4[PML4_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pml4[PML4_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pdpt = (uint64_t *) (pml4[PML4_ENTRY(addr)] & ~0xFFF);

	if (!(pdpt[PDPT_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pdpt[PDPT_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pd = (uint64_t *) (pdpt[PDPT_ENTRY(addr)] & ~0xFFF);

	if (!(pd[PD_ENTRY(addr)] & PAGE_PRESENT)) {
		newPage = 0;
		status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
		if (status != EFI_SUCCESS) { return status; }
		pd[PD_ENTRY(addr)] = newPage | PAGE_WSP;
		uint64_t *temp = (uint64_t *) newPage;
		ZeroMem((void *) temp, 0x1000);
	}

	pt = (uint64_t *) (pd[PD_ENTRY(addr)] & ~0xFFF);
	pt[PT_ENTRY(addr)] = addr | PAGE_WSP;

	return EFI_SUCCESS;
}

EFI_STATUS map_pages(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr,
					 EFI_PHYSICAL_ADDRESS paddr, UINTN pageCount) {
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS newPage = 0;
	EFI_PHYSICAL_ADDRESS physicalAddress = paddr;

#if VERBOSE_REPORTING
	Print(L"Mapping address range...\n\r   paddr: 0x%lx\n\r   vaddr: 0x%lx\n\r   page count: %u\n\r", paddr, vaddr,
		  pageCount);
#endif

	for (uint64_t address = vaddr; address < vaddr + (pageCount * 0x1000); address += 0x1000) {
		uint64_t *pdpt = NULL;
		uint64_t *pd = NULL;
		uint64_t *pt = NULL;

		if (!(pml4[PML4_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pml4[PML4_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pdpt = (uint64_t *) (pml4[PML4_ENTRY(address)] & ~0xFFF);
		if (!(pdpt[PDPT_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pdpt[PDPT_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pd = (uint64_t *) (pdpt[PDPT_ENTRY(address)] & ~0xFFF);
		if (!(pd[PD_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);
			if (status != EFI_SUCCESS) { return status; }
			status = map_page_identity(systemTable, pml4, newPage);
			if (status != EFI_SUCCESS) { return status; }
			pd[PD_ENTRY(address)] = newPage | PAGE_WSP;
			uint64_t *temp = (uint64_t *) newPage;
			ZeroMem((void *) temp, 0x1000);
		}

		pt = (uint64_t *) (pd[PD_ENTRY(address)] & ~0xFFF);
		pt[PT_ENTRY(address)] = physicalAddress | PAGE_WSP;
		physicalAddress += 0x1000;
	}

	return EFI_SUCCESS;
}

EFI_STATUS map_mem(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, MemMap *memMap, LoadedSectionInfo *sectionInfos,
				   UINTN sectionInfoCount) {
	EFI_STATUS status;
	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map; (char *) entry < (char *) memMap->map + memMap->size;
		 entry = (EFI_MEMORY_DESCRIPTOR *) ((char *) entry + memMap->descriptorSize)) {
		switch (entry->Type) {
			case EfiLoaderCode:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiLoaderData:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiMemoryMappedIO:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiMemoryMappedIOPortSpace:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiACPIReclaimMemory:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiACPIMemoryNVS:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiRuntimeServicesCode:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiRuntimeServicesData:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiBootServicesData:
				status = map_pages(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			default:
				break;
		}
		if (status != EFI_SUCCESS) { return status; }
	}

	for (LoadedSectionInfo *sectionInfo = sectionInfos;
		 (char *) sectionInfo < (char *) sectionInfos + sectionInfoCount * sizeof(LoadedSectionInfo);
		 sectionInfo = (LoadedSectionInfo *) ((char *) sectionInfo + sizeof(LoadedSectionInfo))) {
		status = map_pages(systemTable, pml4, sectionInfo->vaddr, sectionInfo->paddr, sectionInfo->pageCount);
		if (status != EFI_SUCCESS) { return status; }
	}

	return EFI_SUCCESS;
}

EFI_STATUS get_EFI_map(EFI_SYSTEM_TABLE *systemTable, MemMap *map) {
	map->map = NULL;
	map->size = 0;

	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize,
																&map->descriptorVersion);
	if (map->size <= 0) { return EFI_BUFFER_TOO_SMALL; }
	map->size += 10 * sizeof(EFI_MEMORY_DESCRIPTOR);
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, map->size, (void **) &map->map);
	if (status != EFI_SUCCESS) { return status; }
	status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize,
													 &map->descriptorVersion);
	return status;
}

void set_CR3_PML4(uint64_t pml4Addr) { __asm__ volatile("mov %0, %%rax; mov %%rax, %%cr3" ::"r"(pml4Addr)); }

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;
	InitializeLib(imageHandle, systemTable);

	LoadedSectionInfo *sectionInfos = NULL;
	UINTN sectionInfoCount = 0;
	Elf64_Ehdr ehdr;

	status = load_kernel(imageHandle, systemTable, &ehdr, &sectionInfoCount, &sectionInfos);
	HandleError(L"Failed to load kernel", status);

	void (*_start)(BootInfo *) = ((__attribute__((sysv_abi)) void (*)(BootInfo *)) ehdr.e_entry);

	EFI_PHYSICAL_ADDRESS pml4Addr;
	uint64_t *pml4 = NULL;
	status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4Addr);
	HandleError(L"Failed to allocate PML4", status);
	pml4 = (uint64_t *) pml4Addr;
	ZeroMem((void *) pml4, 0x1000);

	GOPFramebuffer *framebuffer = NULL;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(framebuffer), (void **) &framebuffer);
	HandleError(L"Failed to allocate memory for framebuffer info struct", status);
	status = init_GOP(systemTable, framebuffer);
	HandleError(L"Failed to initialize GOP", status);

	PSF1_Font *font = NULL;
	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_Font), (void **) &font);
	status = get_PSF1_font(imageHandle, systemTable, L"FONTS\\zap-light16.psf", font);
	HandleError(L"Failed to read PSF font", status);

	MemMap *map = NULL;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(MemMap), (void **) &map);
	HandleError(L"Failed to allocate memory for memory map", status);

	uint64_t *bootstrapHeap = NULL;
	EFI_PHYSICAL_ADDRESS bootstrapHeapAddr;
	status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, BOOTSTRAP_HEAP_PAGE_COUNT,
													  &bootstrapHeapAddr);
	HandleError(L"Failed to allocate bootstrap heap", status);
	bootstrapHeap = (uint64_t *) bootstrapHeapAddr;
	if (!bootstrapHeap) { HandleError(L"Bootstrap heap is nullptr", EFI_LOAD_ERROR); }
	ZeroMem(bootstrapHeap, BOOTSTRAP_HEAP_PAGE_COUNT * 0x1000);

	uint64_t *bootstrapHeapVaddr = 0;

	for (unsigned int i = 0; i < sectionInfoCount; i++) {
		if ((uint64_t *) (sectionInfos[i].vaddr + sectionInfos[i].pageCount * 0x1000) > bootstrapHeapVaddr) {
			bootstrapHeapVaddr = (uint64_t *) (sectionInfos[i].vaddr + sectionInfos[i].pageCount * 0x1000);
		}
	}

#if VERBOSE_REPORTING
	Print(L"Doing mapping pass...\n\r");
#endif
	status = get_EFI_map(systemTable, map);
	HandleError(L"Failed to obtain memory map", status);
	status = map_mem(systemTable, pml4, map, sectionInfos, sectionInfoCount);
	HandleError(L"Failed to map memory", status);

	status = map_pages(systemTable, pml4, (EFI_VIRTUAL_ADDRESS) framebuffer->bufferBase,
					   (EFI_PHYSICAL_ADDRESS) framebuffer->bufferBase, (framebuffer->bufferSize + 0x1000 - 1) / 0x1000);
	HandleError(L"Failed to map memory for the framebuffer", status);
	status = map_pages(systemTable, pml4, (EFI_VIRTUAL_ADDRESS) bootstrapHeapVaddr,
					   (EFI_PHYSICAL_ADDRESS) bootstrapHeap, BOOTSTRAP_HEAP_PAGE_COUNT);
	HandleError(L"Failed to map memory for the bootstrap heap", status);

	BootInfo bootInfo;
	bootInfo.map = map;
	bootInfo.pml4 = pml4;
	bootInfo.bootExtra.framebuffer = framebuffer;
	bootInfo.bootExtra.basicFont = font;
	bootInfo.bootstrapMem.baseAddr = bootstrapHeapVaddr;
	bootInfo.bootstrapMem.topAddr = (uint64_t *) ((uint64_t) bootstrapHeapVaddr + (BOOTSTRAP_HEAP_PAGE_COUNT * 0x1000));
	bootInfo.bootstrapMem.basePaddr = bootstrapHeap;
	bootInfo.bootstrapMem.topPaddr = (uint64_t *) ((uint64_t) bootstrapHeap + (BOOTSTRAP_HEAP_PAGE_COUNT * 0x1000));
	bootInfo.bootstrapMem.size = BOOTSTRAP_HEAP_PAGE_COUNT * 0x1000;

	systemTable->BootServices->FreePool(map->map);
	status = get_EFI_map(systemTable, map);
	HandleError(L"Failed to obtain final memory map", status);

	status = systemTable->BootServices->ExitBootServices(imageHandle, map->key);
	if (status != EFI_SUCCESS) {
		if (status == EFI_INVALID_PARAMETER) {
			int exitStatus = 0;
			for (int i = 0; i < 10; i++) {
				systemTable->BootServices->FreePool(map->map);
				status = get_EFI_map(systemTable, map);
				bootInfo.map = map;
				status = systemTable->BootServices->ExitBootServices(imageHandle, map->key);
				if (status == EFI_SUCCESS) {
					exitStatus = 1;
					break;
				}
			}
			if (!exitStatus) { return status; }
		} else {
			return status;
		}
	}

	set_CR3_PML4((uint64_t) pml4 | PAGE_WSP);
	_start(&bootInfo);

	return EFI_SUCCESS;
}