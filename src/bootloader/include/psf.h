#ifndef BOOT_PSF_H
#define BOOT_PSF_H

#include "../include/basics.h"
#include <efi.h>
#include <efilib.h>

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864ab572

typedef struct {
	uint16_t magic;
	uint8_t fontMode;
	uint8_t charSize;
} PSF1_Header;

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t glyphCount;
	uint32_t bytesPerGlyph;
	uint32_t height;
	uint32_t width;
} PSF2_Header;

typedef struct {
	PSF1_Header *header;
	void *glyphs;
} PSF1_Font;

EFI_STATUS get_PSF1_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path, PSF1_Font *font);
EFI_STATUS get_PSF2_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path);

#endif