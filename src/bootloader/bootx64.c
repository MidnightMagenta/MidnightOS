#include <efi.h>
#include <efilib.h>
#include <elf.h>

#define VERBOSE_REPORTING 1

#define HandleError(fmt, status)                                                                                                                     \
	if (status != EFI_SUCCESS) {                                                                                                                     \
		Print(fmt L": 0x%lx\n\r", status);                                                                                                           \
		return status;                                                                                                                               \
	}
#define ALIGN_ADDR(val, alignment, castType) ((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))

#define PT_ENTRY(addr) (addr >> 12) & 0x1FF
#define PD_ENTRY(addr) (addr >> 21) & 0x1FF
#define PDPT_ENTRY(addr) (addr >> 30) & 0x1FF
#define PML4_ENTRY(addr) (addr >> 39) & 0x1FF

#define PAGE_PRESENT 1 << 0
#define PAGE_WRITABLE 1 << 1
#define PAGE_SUPERVISOR 1 << 2
#define PAGE_WSP PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR

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
	MemMap map;
} BootInfo;

EFI_STATUS OpenFile(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, EFI_FILE **file) {
	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem;
	systemTable->BootServices->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **) &loadedImage);
	systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &fileSystem);

	if (!directory) { fileSystem->OpenVolume(fileSystem, &directory); }
	return directory->Open(directory, file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
}

int CompareMemory(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}

int VerifyElfHeader(Elf64_Ehdr header) {
	if (CompareMemory(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) {//verify the file is ELF
		Print(L"Invalid kernel binary elf magic\n\r");
		return 0;
	}
	if (header.e_ident[EI_CLASS] != ELFCLASS64) {//check if 64 bit
		Print(L"kernel binary is not 64 bit format\n\r");
		return 0;
	}
	if (header.e_ident[EI_DATA] != ELFDATA2LSB) {//check if little endian
		Print(L"kernel binary has incorrect endianness\n\r");
		return 0;
	}
	if (header.e_type != ET_EXEC) {//check if executable
		Print(L"kernel binary is not executable\n\r");
		return 0;
	}
	if (header.e_machine != EM_X86_64) {//check if x86_64
		Print(L"kernel binary not x86_64 architecture\n\r");
		return 0;
	}
	if (header.e_version != EV_CURRENT) {//check if kernel version is current
		Print(L"Invalid kernal binary elf version\n\r");
		return 0;
	}
	return 1;
}

EFI_STATUS GetPhdrs(EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr ehdr, EFI_FILE *elfFile, Elf64_Phdr **phdrs) {
	EFI_STATUS status;
	//allocate a buffer for phdrs
	UINTN phdrBuffSize = ehdr.e_phnum * ehdr.e_phentsize;
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, phdrBuffSize, (void **) phdrs);
	HandleError(L"Failed to allocate phdr buffer", status);
	//read the phdrs from the file
	status = elfFile->Read(elfFile, &phdrBuffSize, *phdrs);
	HandleError(L"Failed to read phdrs", status);
	return EFI_SUCCESS;
}

UINTN CountLoadableSegments(Elf64_Phdr *phdrs, Elf64_Half phnum, Elf64_Half phentsize) {
	UINTN phdrCount = 0;
	for (Elf64_Phdr *phdr = phdrs; (char *) phdr < (char *) phdrs + phnum * phentsize; phdr = (Elf64_Phdr *) ((char *) phdr + phentsize)) {
		if (phdr->p_type == PT_LOAD) { phdrCount++; }
	}
	return phdrCount;
}

EFI_STATUS LoadKernel(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr *pEhdr, UINTN *sectionInfoCount,
					  LoadedSectionInfo **sectionInfos) {
	EFI_STATUS status;
	//Open kernel.elf
	EFI_FILE *kernel = NULL;
	status = OpenFile(NULL, L"\\boot\\kernel.elf", imageHandle, systemTable, &kernel);
	HandleError(L"Failed to open kernel file: 0x%lx\n\r", status);
	if (!kernel) {
		Print(L"Kernel is nullptr\n\r");
		return EFI_LOAD_ERROR;
	}
	//obtain the elf header
	Elf64_Ehdr ehdr;
	UINTN ehdrSize = sizeof(ehdr);
	kernel->Read(kernel, &ehdrSize, &ehdr);
	if (!VerifyElfHeader(ehdr)) { return EFI_LOAD_ERROR; }
	*pEhdr = ehdr;

	//obtain the phdrs
	Elf64_Phdr *phdrs = NULL;
	status = GetPhdrs(systemTable, ehdr, kernel, &phdrs);
	if (status != EFI_SUCCESS) { return status; }
	UINTN phdrCount = CountLoadableSegments(phdrs, ehdr.e_phnum, ehdr.e_phentsize);
	*sectionInfoCount = phdrCount;

	//allocate sufficient memory for section infos
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, phdrCount * sizeof(LoadedSectionInfo), (void **) sectionInfos);
	if (status != EFI_SUCCESS) { return status; }

	//load the kernel sections into memory
	{
		Elf64_Phdr *phdr = phdrs;
		LoadedSectionInfo *sectionInfo = *sectionInfos;

		while ((char *) phdr < (char *) phdrs + ehdr.e_phnum * ehdr.e_phentsize &&
			   (char *) sectionInfo < (char *) *sectionInfos + phdrCount * sizeof(LoadedSectionInfo)) {
			if (phdr->p_type == PT_LOAD) {//check if the segment is loadable
				//calculate and allocate the required number of pages for the segment
				int pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pageCount, &sectionInfo->paddr);
				if (status != EFI_SUCCESS) { return status; }

				//read the segment from the file
				status = kernel->SetPosition(kernel, phdr->p_offset);
				if (status != EFI_SUCCESS) { return status; }
				UINTN size = phdr->p_memsz;
				status = kernel->Read(kernel, &size, (void *) sectionInfo->paddr);

				if (status != EFI_SUCCESS) { return status; }

				//put required data into the section info structures for mapping later
				sectionInfo->vaddr = phdr->p_vaddr;
				sectionInfo->pageCount = pageCount;
				sectionInfo->flags = phdr->p_flags;

				Print(L"Loaded segment\n\r\t at paddr: 0x%lx\n\r\t expected vaddr: 0x%lx\n\r\t "
					  L"with flags: 0x%x\n\r\t number of pages for segment: %d\n\r",
					  sectionInfo->paddr, sectionInfo->vaddr, sectionInfo->flags, sectionInfo->pageCount);

				//increment section info pointer to the next entry
				sectionInfo = (LoadedSectionInfo *) ((char *) sectionInfo + sizeof(LoadedSectionInfo));
			}
			//increment phdr pointer to the next entry
			phdr = (Elf64_Phdr *) ((char *) phdr + ehdr.e_phentsize);
		}
	}

	return EFI_SUCCESS;
}

EFI_STATUS CreateMap(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, EFI_VIRTUAL_ADDRESS vaddr, EFI_PHYSICAL_ADDRESS paddr, UINTN pageCount) {
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS newPage = 0;
	EFI_VIRTUAL_ADDRESS physicalAddress = paddr;

	//itterate over the address space in increments of 1 page
	for (uint64_t address = vaddr; address < vaddr + (pageCount * 0x1000); address += 0x1000) {
		//predefine paging structure pointers
		uint64_t *pdpt = NULL;
		uint64_t *pd = NULL;
		uint64_t *pt = NULL;

		//verify that the required PDPT is present in PML4, if not, allocate memory for it.
		if (!(pml4[PML4_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);//allocate memory for the structure
			if (status != EFI_SUCCESS) { return status; }
			pml4[PML4_ENTRY(address)] = newPage | PAGE_WSP;//write the pointer to PDPT into PML4
			uint64_t *temp = (uint64_t *) newPage;		   //clear memory pointer to by the structure
			ZeroMem((void *) temp, 0x1000);
		}

		pdpt = (uint64_t *) (pml4[PML4_ENTRY(address)] & ~0xFFF);//convert the PML4 entry to a PDPT pointer
		//verify that the required PD is present in PDPT, if not, allocate memory for it.
		if (!(pdpt[PDPT_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);//allocate memory for the structure
			if (status != EFI_SUCCESS) { return status; }
			pdpt[PDPT_ENTRY(address)] = newPage | PAGE_WSP;//write the pointer to PD into PDPT
			uint64_t *temp = (uint64_t *) newPage;		   //clear memory pointer to by the structure
			ZeroMem((void *) temp, 0x1000);
		}

		pd = (uint64_t *) (pdpt[PDPT_ENTRY(address)] & ~0xFFF);//convert the PDPT entry to a PD pointer
		//verify that the required PT is present in PD, if not, allocate memory for it.
		if (!(pd[PD_ENTRY(address)] & PAGE_PRESENT)) {
			newPage = 0;
			status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &newPage);//allocate memory for the structure
			if (status != EFI_SUCCESS) { return status; }
			pd[PD_ENTRY(address)] = newPage | PAGE_WSP;//write the pointer to PT into PD
			uint64_t *temp = (uint64_t *) newPage;	   //clear memory pointer to by the structure
			ZeroMem((void *) temp, 0x1000);
		}

		pt = (uint64_t *) (pd[PD_ENTRY(address)] & ~0xFFF);//convert the PD entry to a PT pointer
		pt[PT_ENTRY(address)] = physicalAddress | PAGE_WSP;//write the page base address into PDPT as a supervisor read write page
		physicalAddress += 0x1000;
	}

	return EFI_SUCCESS;
}

EFI_STATUS MapMemory(EFI_SYSTEM_TABLE *systemTable, uint64_t *pml4, MemMap *memMap, LoadedSectionInfo *sectionInfos, UINTN sectionInfoCount) {
	EFI_STATUS status;
	//itterate over memory map entries, and pass relavent sections to be mapped
	for (EFI_MEMORY_DESCRIPTOR *entry = memMap->map; (char *) entry < (char *) memMap->map + memMap->size;
		 entry = (EFI_MEMORY_DESCRIPTOR *) ((char *) entry + memMap->descriptorSize)) {
		switch (entry->Type) {//only create a map for relavent structures
			case EfiLoaderCode:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiLoaderData:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiMemoryMappedIO:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiMemoryMappedIOPortSpace:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiACPIReclaimMemory:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiACPIMemoryNVS:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiRuntimeServicesCode:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiRuntimeServicesData:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			case EfiBootServicesData:
				status = CreateMap(systemTable, pml4, entry->PhysicalStart, entry->PhysicalStart, entry->NumberOfPages);
				break;
			default:
				break;
		}
		if (status != EFI_SUCCESS) { return status; }
	}

	for (LoadedSectionInfo *sectionInfo = sectionInfos; sectionInfo < sectionInfos + sizeof(LoadedSectionInfo) * sectionInfoCount;
		 sectionInfo = sectionInfo + sizeof(LoadedSectionInfo)) {
		CreateMap(systemTable, pml4, sectionInfo->vaddr, sectionInfo->paddr, sectionInfo->pageCount);
	}

	return EFI_SUCCESS;
}

EFI_STATUS GetMap(EFI_SYSTEM_TABLE *systemTable, MemMap *map) {
	map->map = NULL;
	map->size = 0;

	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize, &map->descriptorVersion);
	if (map->size <= 0) { return EFI_BUFFER_TOO_SMALL; }
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, map->size + 10 * sizeof(EFI_MEMORY_DESCRIPTOR), (void **) &map->map);
	if (status != EFI_SUCCESS) { return status; }
	status = systemTable->BootServices->GetMemoryMap(&map->size, map->map, &map->key, &map->descriptorSize, &map->descriptorVersion);
	return status;
}

void SetCR3PML4(uint64_t pml4Addr) { __asm__ volatile("mov %0, %%rax; mov %%rax, %%cr3" ::"r"(pml4Addr)); }

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;
	InitializeLib(imageHandle, systemTable);

	//structues obtained from loading the kernel file
	LoadedSectionInfo *sectionInfos = NULL;
	UINTN sectionInfoCount = 0;
	Elf64_Ehdr ehdr;

	//load the kernel into memory, and retrieve necessary structures
	status = LoadKernel(imageHandle, systemTable, &ehdr, &sectionInfoCount, &sectionInfos);
	HandleError(L"Failed to load kernel", status);

	//find the entry point of the kernel
	void (*_start)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*)) ehdr.e_entry);
	Print(L"Found entry symbol at 0x%lx\n\r", (Elf64_Addr) _start);

	//allocate memory for PML4 (1 page, 512 entries)
	EFI_PHYSICAL_ADDRESS pml4Addr;
	uint64_t *pml4 = NULL;
	status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4Addr);
	HandleError(L"Failed to allocate PML4", status);
	pml4 = (uint64_t *) pml4Addr;
	ZeroMem((void *) pml4, 0x1000);

	//obtain the memory map to prepare the paging structures
	MemMap map;
	status = GetMap(systemTable, &map);
	HandleError(L"Failed to obtain memory map", status);

	status = MapMemory(systemTable, pml4, &map, sectionInfos, sectionInfoCount);
	HandleError(L"Failed to map memory", status);

	Print(L"pml4 addr: 0x%lx\n\r", pml4);

	status = GetMap(systemTable, &map);//get final memory map
	HandleError(L"Failed to obtain memory map", status);

	EFI_PHYSICAL_ADDRESS addr = 0x00000FFE00000;
	uint64_t *pdpe = (uint64_t *) (pml4[PML4_ENTRY(addr)] & ~0xFFF);

	BootInfo bootInfo;
	bootInfo.map = map;

	status = systemTable->BootServices->ExitBootServices(imageHandle, map.key);//exit boot services
	if (status != EFI_SUCCESS) { return status; }

	SetCR3PML4((uint64_t) pml4 | PAGE_WSP);

	_start(&bootInfo);

	return EFI_SUCCESS;
}