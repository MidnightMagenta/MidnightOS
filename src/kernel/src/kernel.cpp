#include "../include/boot_info.hpp"

extern "C" void kernel_entry(BootInfo* bootInfo) {
    unsigned int y = 50;
	unsigned int bbp = 4;
    GOPFramebuffer *framebuffer = bootInfo->framebuffer;
	for(unsigned int x = 0; x < framebuffer->width / 2 * bbp; x++){
		*(unsigned int*)(x + (y * framebuffer->pixelsPerScanline * bbp) + (char*)framebuffer->bufferBase) = 0xFFFFFFFF;
	}
	__asm__("hlt");
	return;
}