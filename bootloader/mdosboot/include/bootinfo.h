#ifndef MDOSBOOT_BOOTINFO_H
#define MDOSBOOT_BOOTINFO_H

#include "../include/elf.h"
#include "../include/types.h"
#include <abi/boot/boot_info.h>
#include <efi.h>
#include <efilib.h>

typedef struct {
	elf_loadinfo_t *elfSections;
} bi_bootinfo_createinfo;

EFI_STATUS bi_build_bootinfo(bi_bootinfo_createinfo *createInfo, bi_bootinfo_t** bootInfo);
EFI_STATUS bi_update_memmap(const efi_memmap_t* const efiMap, bi_memmap_t *memmap);

#endif