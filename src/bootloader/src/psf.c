#include "../include/psf.h"

EFI_STATUS get_PSF1_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path, PSF1_Font *font) {
	EFI_FILE *fontFile = NULL;
	EFI_STATUS status = open_file(NULL, path, imageHandle, systemTable, &fontFile);
	HandleError(L"Failed to open font file", status);

	UINTN bytesToRead = 4;
	uint32_t PSF_Magic;
	fontFile->Read(fontFile, &bytesToRead, &PSF_Magic);

	if ((PSF_Magic & 0xFFFF) != PSF1_MAGIC) {
		Print(L"Failed to verify font magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	}

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_Header), (void **) &font->header);
	HandleError(L"Failed to allocate font header memory", status);
	fontFile->SetPosition(fontFile, 0);
	bytesToRead = sizeof(PSF1_Header);
	fontFile->Read(fontFile, &bytesToRead, font->header);

	UINTN glyphBufferSize = font->header->charSize * 256;
	if (font->header->fontMode == 1) { glyphBufferSize = font->header->charSize * 512; }

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void **) &font->glyphs);
	HandleError(L"Failed to allocate font buffer memory", status);
	fontFile->SetPosition(fontFile, sizeof(PSF1_Header));
	fontFile->Read(fontFile, &glyphBufferSize, font->glyphs);

	fontFile->Close(fontFile);
	return EFI_SUCCESS;
}

EFI_STATUS get_PSF2_font(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable, CHAR16 *path) {
	EFI_FILE *fontFile = NULL;
	EFI_STATUS status = open_file(NULL, path, imageHandle, systemTable, &fontFile);
	HandleError(L"Failed to open font file", status);

	UINTN bytesToRead = 4;
	uint32_t PSF_Magic;
	fontFile->Read(fontFile, &bytesToRead, &PSF_Magic);

	if (PSF_Magic == PSF2_MAGIC) {
		//read PSF2 font
		//TODO: implement PSF2 font reading
		Print(L"PSF2 fonts not supported. Magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Failed to verify font magic: 0x%x\n\r", PSF_Magic);
		fontFile->Close(fontFile);
		return EFI_LOAD_ERROR;
	}
}