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
	Elf64_Addr phys; /// The physical address into which the section was loaded
	Elf64_Addr reqVirt; /// The virtual address specified in the program header
	size_t pageCount; /// The number of 4 KiB pages the section uses
	Elf32_Word flags; /// ELF program section flags
} elf_sectioninfo_t;

/**
 * \brief Post load information needed to map and execute the loaded binary.
 */
typedef struct {
	Elf64_Addr entry; /// The executable's entry point address
	size_t sectionCount; /// The number of elf_sectioninfo_t structures in the sections buffer
	elf_sectioninfo_t *sections; /// The buffer containing loaded section information
} elf_loadinfo_t;

/**
 * \brief ELF program sections load strategies.
 */
typedef enum {
	ELF_LOAD_AUTO, /// Attempts to load sections in a contigous fashion first. If failed, falls back to discontigous
	ELF_LOAD_CONTIG, /// Loads program sections one after another in memory in the enumeration order of program headers, into a flat image.
	ELF_LOAD_DISCONTIG, /// Loads program sections into any location in memory
} elf_loadtype_t;

/**
 * \brief Loads an ELF binary into memory at any free address.
 * \param file pointer to EFI_FILE containing the ELF binary to load.
 * \param info pointer to elf_loadingo_t structure containing post-load informations.
 */
EFI_STATUS elf_load_file(EFI_FILE *file, elf_loadinfo_t *info, elf_loadtype_t loadType);

/**
 * \brief Frees the elf_loadinfo_t structure.
 * \param info elf_loadinfo_t pointer to the structure to free.
 */
void elf_free_loadinfo(elf_loadinfo_t *info);

/**
 * \brief Frees the memory used to load the section data.
 * \param info elf_loadinfo_t pointer to loaded section information.
 */
void elf_free_sections(elf_loadinfo_t *info);

/**
 * \brief Prints debug information about the loaded sections.
 * \param info elf_loadinfo_t pointer to loaded section information.
 */
void elf_print_sections(const elf_loadinfo_t *info);

/** @} */

#endif