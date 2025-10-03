#include "../include/elf.h"
#include "../include/debug.h"

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

EFI_STATUS elf_load_file(EFI_FILE *file, elf_loadinfo_t *info) {
	EFI_STATUS res = EFI_SUCCESS;

	// get and verify the elf header
	Elf64_Ehdr ehdr;
	UINTN ehdrSz = sizeof(Elf64_Ehdr);
	res = file->Read(file, &ehdrSz, &ehdr);
	if (EFI_ERROR(res)) { return res; }

	res = elf_verify_ehdr(&ehdr);
	if (EFI_ERROR(res)) { return res; }

	info->entry = ehdr.e_entry;

	// get the program headers
	Elf64_Phdr *phdrs = NULL;
	UINTN phdrBufferSize = ehdr.e_phnum * ehdr.e_phentsize;
	res = gBS->AllocatePool(EfiLoaderData, phdrBufferSize, (void **) &phdrs);
	if (EFI_ERROR(res)) { return res; }

	res = file->SetPosition(file, ehdr.e_phoff);
	if (EFI_ERROR(res)) {
		goto cleanup_1;
		return res;
	}
	res = file->Read(file, &phdrBufferSize, phdrs);
	if (EFI_ERROR(res)) {
		goto cleanup_1;
		return res;
	}

	// count how many sections to load, and allocate memory for section infos
	info->sectionCount = elf_count_loadable(&ehdr, phdrs);
	DBG_MSG("Loadable count: %d\n\r", info->sectionCount);

	res = gBS->AllocatePool(EfiLoaderData, info->sectionCount * sizeof(elf_sectioninfo_t), (void **) &info->sections);
	if (EFI_ERROR(res)) {
		goto cleanup_1;
		return res;
	}
	ZeroMem(info->sections, info->sectionCount * sizeof(elf_sectioninfo_t));

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
			if (EFI_ERROR(res)) {
				goto cleanup_2;
				return res;
			}

			// zero out the memory before loading
			ZeroMem((void *) paddr, pageCount * 0x1000);

			// load the section's contents
			res = file->SetPosition(file, phdr->p_offset);
			if (EFI_ERROR(res)) {
				goto cleanup_2;
				return res;
			}
			UINTN size = phdr->p_filesz;
			res = file->Read(file, &size, (void *) paddr);
			if (EFI_ERROR(res)) {
				goto cleanup_2;
				return res;
			}

			// get information from the phdr
			loadedSection->pageCount = pageCount;
			loadedSection->phys = paddr;
			loadedSection->reqVirt = phdr->p_vaddr;

			// increment the section info pointer
			loadedSection = (elf_sectioninfo_t *) ((char *) loadedSection + sizeof(elf_sectioninfo_t));
		}

		phdrIndex++;
	}

cleanup_2:
	info->sectionCount = 0;
	gBS->FreePool(info->sections);
cleanup_1:
	gBS->FreePool(phdrs);

	return res;
}