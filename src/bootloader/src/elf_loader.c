#include "../include/elf_loader.h"

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
				UINTN pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pageCount,
																  &sectionInfo->paddr);
				if (status != EFI_SUCCESS) { return status; }
				ZeroMem((void *) sectionInfo->paddr, pageCount * 0x1000);

				status = kernel->SetPosition(kernel, phdr->p_offset);
				if (status != EFI_SUCCESS) { return status; }
				UINTN size = phdr->p_filesz;
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