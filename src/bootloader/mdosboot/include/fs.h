#ifndef MDOSBOOT_FS_H
#define MDOSBOOT_FS_H

#include <efi.h>
#include <efilib.h>

/** \defgroup bootloader_internal Bootloader Internals 
 *  @brief Internal bootloader functions.
 * 
 * 	Internal functions available to the bootloader.
 *  They may be used to extend the bootloader,
 *  but are not guaranteed to be avaiable globally.
 * 
 *  @{
*/

/** \brief Finds EFI_SIMPLE_FILE_SYSTEM_PROTOCOL for a partition with a given GUID
 *  \param guid GUID being searched for
 *  \param handle Returned EFI_HANDLE to the found partition.
 *  \param filesystem Returned EFI_SIMPLE_FILE_SYSTEM_PROTOCOL for the partition
 *  \return The status of the function
 */
EFI_STATUS EFIAPI find_filesystem_for_guid(IN EFI_GUID *guid,
									OUT EFI_HANDLE *handle,
									OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **filesystem);

/** \brief Finds EFI_SIMPLE_FILE_SYSTEM_PROTOCOL for a partition that contains a given file
 *  \param path Path to a file being searched for
 *  \param handle Returned EFI_HANDLE to the found partition.
 *  \param filesystem Returned EFI_SIMPLE_FILE_SYSTEM_PROTOCOL for the partition
 *  \return The status of the function
 */
EFI_STATUS EFIAPI find_filesystem_with_file(IN CHAR16 *path,
									 OUT EFI_HANDLE *handle,
									 OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **filesystem);

/** \brief Opens an EFI_FILE_PROTOCOL for a given path
 *  \param filesystem EFI_SIMPLE_FILE_SYSTEM_PROTOCOL on which the file is located. Can be NULL if root is specified
 *  \param root The root directory from which to open the file. Can be NULL if filesystem is specified
 *  \param path Path to the file to open
 *  \param openMode EFI open mode for the file
 *  \param attributes EFI attributes for file creation (should be 0 if openMode does not contain EFI_FILE_MODE_CREATE)
 *  \param file The returned EFI_FILE_PROTOCOL for a file
 *  \return The status of the function
 */
EFI_STATUS EFIAPI open_file(IN OPTIONAL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem,
					 IN OPTIONAL EFI_FILE *root,
					 IN CHAR16 *path,
					 IN UINT64 openMode,
					 IN OPTIONAL UINT64 attributes,
					 OUT EFI_FILE **file);

/** @} */

#endif