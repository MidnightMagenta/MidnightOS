#ifndef MDOSBOOT_ELF_H
#define MDOSBOOT_ELF_H

#include <efi.h>
#include <efilib.h>
#include <elf.h>

/** 
 * \defgroup bootloader_internal Bootloader Internals 
 * @brief Internal bootloader functions.
 * 
 * Internal functions available to the bootloader.
 * They may be used to extend the bootloader,
 * but are not guaranteed to be avaiable globally.
 * 
 * @{
*/

/**
 * \brief Loaded section information containing the physical address it was loaded to, it's requested virtual address, and size in memory.
 */
typedef struct {
	Elf64_Addr phys;
	Elf64_Addr reqVirt;
	size_t pageCount;
} elf_sectioninfo_t;

/**
 * \brief Post load information needed to map and execute the loaded binary.
 */
typedef struct {
	Elf64_Addr entry;
	size_t sectionCount;
	elf_sectioninfo_t *sections;
} elf_loadinfo_t;

/**
 * \brief Loads an ELF binary into memory at any free address.
 * \param file pointer to EFI_FILE containing the ELF binary to load.
 * \param info pointer to elf_loadingo_t structure containing post-load informations.
 */
EFI_STATUS elf_load_file(EFI_FILE *file, elf_loadinfo_t *info);

/** @} */

#endif