#include "../include/kernel.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	renderer.Initialize(bootInfo->framebuffer->bufferBase, bootInfo->framebuffer->bufferSize, bootInfo->framebuffer->width,
						bootInfo->framebuffer->height, bootInfo->framebuffer->pixelsPerScanline);
	renderer.ClearBuffer(MAKE_COLOR(25, 25, 25, 255));
}