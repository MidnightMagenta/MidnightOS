#ifndef MDOSBOOT_CFGPARSE_H
#define MDOSBOOT_CFGPARSE_H

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

#include "../include/crc32.h"
#include <efi.h>
#include <efilib.h>

typedef struct {
    EFI_GUID bootPartUUID;
    CHAR16 *bootBinPath;
} configinfo_t;

static const uint8_t MDBC_Magic[4] = {'M', 'B', 'C', 'F'};

typedef struct {
    uint8_t magic[4];
    crc32_t crc32;
    uint8_t version;
    uint8_t pad[7];
    uint64_t dataOffset;
} __attribute__((packed)) mdbc_header_t;

/**
 * \brief Parses boot configuration file and returns boot config information
 * 
 * The configuration file is specified using the Midnight Boot Binary Configuration file format.
 * The configuration file must be located at the path boot/boot.cfg in the partition the bootloader 
 * image was loaded from.
 * 
 * Required options:
 * 
 *  - BOOT_DISK - of type GUID, the GUID of the partitions on which the kernel image to load resides
 * 
 *  - BOOT_PATH - of type UTF16, the path to the kernel image to load
 * 
 * \param imageHandle EFI_HANDLE to the loaded image
 * \param cfg The returned configuration info
 */
EFI_STATUS parse_config(IN EFI_HANDLE imageHandle, OUT configinfo_t *cfg);

/**
 * \brief Frees boot configuration info structure
 * \param cfg The configuration info structure
 */
void free_config(IN configinfo_t *cfg);

/** @} */

#endif