#ifndef MDBOOT_BOOT_GOP_H
#define MDBOOT_BOOT_GOP_H

#include "../include/basics.h"
#include <efi.h>
#include <efilib.h>

typedef struct {
	void *bufferBase;
	UINTN bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanline;
} GOPFramebuffer;

EFI_STATUS init_GOP(EFI_SYSTEM_TABLE *systemTable, GOPFramebuffer *framebuffer);

#endif