#include "../include/kernel.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	m_renderer.Initialize(bootInfo->bootExtra.framebuffer->bufferBase, bootInfo->bootExtra.framebuffer->bufferSize, bootInfo->bootExtra.framebuffer->width,
						  bootInfo->bootExtra.framebuffer->height, bootInfo->bootExtra.framebuffer->pixelsPerScanline);
	m_renderer.ClearBuffer(MAKE_COLOR(25, 25, 25, 255));

	m_tty.Initialize(&m_renderer, bootInfo->bootExtra.basicFont);
	m_tty.PrintString("Test String\nAnother string", 26);
}