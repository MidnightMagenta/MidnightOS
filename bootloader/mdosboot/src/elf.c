#include "../include/elf.h"
#include "../include/debug.h"
#include "../include/utils.h"

static EFI_STATUS elf_verify_ehdr(const Elf64_Ehdr *ehdr) {
	if (CompareMem(&ehdr->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) { return EFI_COMPROMISED_DATA; }
	if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) { return EFI_UNSUPPORTED; }
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) { return EFI_UNSUPPORTED; }
	if (ehdr->e_type != ET_EXEC) { return EFI_UNSUPPORTED; }
	if (ehdr->e_machine != EM_X86_64) { return EFI_UNSUPPORTED; }
	if (ehdr->e_version != EV_CURRENT) { return EFI_UNSUPPORTED; }

	return EFI_SUCCESS;
}

static UINTN elf_count_loadable(const Elf64_Ehdr *ehdr, const Elf64_Phdr *phdrs) {
	UINTN count = 0;
	for (size_t i = 0; i < ehdr->e_phnum; i++) {
		Elf64_Phdr *phdr = (Elf64_Phdr *) ((char *) phdrs + ehdr->e_phentsize * i);
		if (phdr->p_type == PT_LOAD) { count++; }
	}
	return count;
}

static UINTN elf_get_section_size(const Elf64_Ehdr *ehdr, const Elf64_Phdr *phdrs) {
	UINTN pageCount = 0;
	for (size_t i = 0; i < ehdr->e_phnum; i++) {
		Elf64_Phdr *phdr = (Elf64_Phdr *) ((char *) phdrs + ehdr->e_phentsize * i);
		if (phdr->p_type == PT_LOAD) { pageCount += ALIGN_UP(phdr->p_memsz, 0x1000, UINTN) / 0x1000; }
	}
	return pageCount;
}

static EFI_STATUS elf_get_file_info(EFI_FILE *file, elf_loadinfo_t *info, Elf64_Ehdr *ehdr, Elf64_Phdr **phdrs) {
	EFI_STATUS res = EFI_SUCCESS;

	// get and verify the elf header
	UINTN ehdrSz = sizeof(Elf64_Ehdr);
	res = file->Read(file, &ehdrSz, ehdr);
	if (EFI_ERROR(res)) { return res; }

	res = elf_verify_ehdr(ehdr);
	if (EFI_ERROR(res)) { return res; }

	info->entry = ehdr->e_entry;

	// get the program headers
	UINTN phdrBufferSize = ehdr->e_phnum * ehdr->e_phentsize;
	res = gBS->AllocatePool(EfiLoaderData, phdrBufferSize, (void **) phdrs);
	if (EFI_ERROR(res)) { return res; }

	res = file->SetPosition(file, ehdr->e_phoff);
	if (EFI_ERROR(res)) {
		gBS->FreePool(*phdrs);
		return res;
	}
	res = file->Read(file, &phdrBufferSize, *phdrs);
	if (EFI_ERROR(res)) {
		gBS->FreePool(*phdrs);
		return res;
	}

	// count how many sections to load, and allocate memory for section infos
	info->sectionCount = elf_count_loadable(ehdr, *phdrs);
	res = gBS->AllocatePool(EfiLoaderData, info->sectionCount * sizeof(elf_sectioninfo_t), (void **) &info->sections);
	if (EFI_ERROR(res)) {
		gBS->FreePool(*phdrs);
		return res;
	}
	ZeroMem(info->sections, info->sectionCount * sizeof(elf_sectioninfo_t));

	return EFI_SUCCESS;
}

static EFI_STATUS elf_load_file_discont(EFI_FILE *file, elf_loadinfo_t *info) {
	EFI_STATUS res = EFI_SUCCESS;

	Elf64_Ehdr ehdr;
	Elf64_Phdr *phdrs;

	res = elf_get_file_info(file, info, &ehdr, &phdrs);
	if (EFI_ERROR(res)) { return res; }

	// load the sections
	size_t phdrIndex = 0;
	elf_sectioninfo_t *loadedSection = info->sections;
	while (phdrIndex < ehdr.e_phnum) {
		Elf64_Phdr *phdr = (Elf64_Phdr *) ((char *) phdrs + ehdr.e_phentsize * phdrIndex);
		if (phdr->p_type == PT_LOAD) {
			// allocate memory for the sections
			Elf64_Addr paddr = 0;
			UINTN pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, pageCount, &paddr);
			if (EFI_ERROR(res)) { goto err_free_loadInfo; }

			// zero out the memory before loading
			ZeroMem((void *) paddr, pageCount * 0x1000);

			// load the section's contents
			res = file->SetPosition(file, phdr->p_offset);
			if (EFI_ERROR(res)) { goto err_free_loadInfo; }
			UINTN size = phdr->p_filesz;
			res = file->Read(file, &size, (void *) paddr);
			if (EFI_ERROR(res)) { goto err_free_loadInfo; }

			// get information from the phdr
			loadedSection->pageCount = pageCount;
			loadedSection->phys = paddr;
			loadedSection->reqVirt = phdr->p_vaddr;
			loadedSection->flags = phdr->p_flags;

			// increment the section info pointer
			loadedSection++;
		}

		phdrIndex++;
	}

	return EFI_SUCCESS;

err_free_loadInfo:
	elf_free_loadinfo(info);
	gBS->FreePool(phdrs);

	return res;
}

static EFI_STATUS elf_load_file_cont(EFI_FILE *file, elf_loadinfo_t *info) {
	EFI_STATUS res = EFI_SUCCESS;

	Elf64_Ehdr ehdr;
	Elf64_Phdr *phdrs;

	res = elf_get_file_info(file, info, &ehdr, &phdrs);
	if (EFI_ERROR(res)) { return res; }

	UINTN pageCount = elf_get_section_size(&ehdr, phdrs);
	Elf64_Addr loadBuffer = 0;
	res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, pageCount, &loadBuffer);
	if (EFI_ERROR(res)) { goto err_free_loadInfo; }
	ZeroMem((void *) loadBuffer, pageCount * 0x1000);

	// load the sections
	size_t phdrIndex = 0;
	elf_sectioninfo_t *loadedSection = info->sections;
	Elf64_Addr paddr = loadBuffer;
	while (phdrIndex < ehdr.e_phnum) {
		Elf64_Phdr *phdr = (Elf64_Phdr *) ((char *) phdrs + ehdr.e_phentsize * phdrIndex);
		if (phdr->p_type == PT_LOAD) {
			// load the section's contents
			res = file->SetPosition(file, phdr->p_offset);
			if (EFI_ERROR(res)) { goto err_free_loadBuffer; }
			UINTN size = phdr->p_filesz;
			res = file->Read(file, &size, (void *) paddr);
			if (EFI_ERROR(res)) { goto err_free_loadBuffer; }

			// get information from the phdr
			loadedSection->pageCount = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			loadedSection->phys = paddr;
			loadedSection->reqVirt = phdr->p_vaddr;
			loadedSection->flags = phdr->p_flags;

			// increment the section info pointer
			paddr += ALIGN_UP(phdr->p_memsz, 0x1000, Elf64_Addr);
			loadedSection++;
		}

		phdrIndex++;
	}

	return EFI_SUCCESS;

err_free_loadBuffer:
	gBS->FreePages(loadBuffer, pageCount);
err_free_loadInfo:
	elf_free_loadinfo(info);
	gBS->FreePool(phdrs);

	return res;
}

EFI_STATUS elf_load_file(EFI_FILE *file, elf_loadinfo_t *info, elf_loadtype_t loadType) {
	switch (loadType) {
		case ELF_LOAD_AUTO: {
			EFI_STATUS res = elf_load_file_cont(file, info);
			if (EFI_ERROR(res)) { return elf_load_file_discont(file, info); }
			return res;
		}
		case ELF_LOAD_CONTIG:
			return elf_load_file_cont(file, info);
		case ELF_LOAD_DISCONTIG:
			return elf_load_file_discont(file, info);
		default:
			return EFI_INVALID_PARAMETER;
	}
}

void elf_free_loadinfo(elf_loadinfo_t *info) {
	if (info->sections != NULL) { gBS->FreePool(info->sections); }
	info->sectionCount = 0;
	info->entry = 0;
}

void elf_free_sections(elf_loadinfo_t *info) {
	for (size_t i = 0; i < info->sectionCount; i++) {
		elf_sectioninfo_t *section = (elf_sectioninfo_t *) ((char *) info->sections + i * sizeof(elf_sectioninfo_t));
		gBS->FreePages(section->phys, section->pageCount);
	}
}

void elf_print_sections(const elf_loadinfo_t *info) {
	for (size_t i = 0; i < info->sectionCount; i++) {
		elf_sectioninfo_t *section = (elf_sectioninfo_t *) ((char *) info->sections + i * sizeof(elf_sectioninfo_t));
		DBG_MSG("Elf section %d\n\r paddr: 0x%lx\n\r vaddr: 0x%lx\n\r page count: %d\n\r Flags: ", i, section->phys,
						section->reqVirt, section->pageCount);
		if (section->flags & PF_X) { DBG_MSG("X"); }
		if (section->flags & PF_R) { DBG_MSG("R"); }
		if (section->flags & PF_W) { DBG_MSG("W"); }
		DBG_MSG("\n\r");
	}
}