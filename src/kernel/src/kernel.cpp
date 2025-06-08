#include "../include/kernel.hpp"
#include "../include/IO/kprint.hpp"
#include "../include/kstring.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	g_renderer.Initialize(bootInfo->bootExtra.framebuffer->bufferBase, bootInfo->bootExtra.framebuffer->bufferSize,
						  bootInfo->bootExtra.framebuffer->width, bootInfo->bootExtra.framebuffer->height,
						  bootInfo->bootExtra.framebuffer->pixelsPerScanline);
	g_renderer.ClearBuffer(MAKE_COLOR(25, 25, 25, 255));
	MdOS::IO::kprintSystem::Initialize(&g_renderer, bootInfo->bootExtra.basicFont);

	MdOS::IO::kprint("Test string %d %f %x %lx", 10, 12.12241f, 0xABCD, (uint64_t) 0x0123456789ABCDEF);
}