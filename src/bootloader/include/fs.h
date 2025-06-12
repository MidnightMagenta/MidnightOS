#ifndef BOOT_FS_H
#define BOOT_FS_H

#include <efi.h>
#include <efilib.h>
#include "../include/basics.h"

EFI_STATUS open_file(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable,
    EFI_FILE **file);

#endif