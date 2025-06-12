#ifndef BOOT_ELF_LOADER_H
#define BOOT_ELF_LOADER_H

#include "../include/basics.h"
#include "../include/fs.h"
#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef struct {
	Elf64_Addr paddr;
	Elf64_Addr vaddr;
	UINTN pageCount;
	Elf64_Word flags;
} LoadedSectionInfo;

int verify_ehdr(Elf64_Ehdr header);
EFI_STATUS get_phdrs(EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr ehdr, EFI_FILE *elfFile, Elf64_Phdr **phdrs);
UINTN count_loadable_segments(Elf64_Phdr *phdrs, Elf64_Half phnum, Elf64_Half phentsize);
EFI_STATUS load_kernel(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, Elf64_Ehdr *pEhdr,
					   UINTN *sectionInfoCount, LoadedSectionInfo **sectionInfos);

#endif