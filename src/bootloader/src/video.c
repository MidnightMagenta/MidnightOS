#include "../include/video.h"

EFI_STATUS init_GOP(EFI_SYSTEM_TABLE *systemTable, GOPFramebuffer *framebuffer) {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	status = systemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);
	HandleError(L"Failed to locate GOP", status);

	framebuffer->bufferBase = (void *) gop->Mode->FrameBufferBase;
	framebuffer->bufferSize = gop->Mode->FrameBufferSize;
	framebuffer->width = gop->Mode->Info->HorizontalResolution;
	framebuffer->height = gop->Mode->Info->VerticalResolution;
	framebuffer->pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

#if VERBOSE_REPORTING
	Print(L"GOP initialized...\n\r");
#endif

	return EFI_SUCCESS;
}