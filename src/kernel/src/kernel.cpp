#include "../include/kernel.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	m_renderer.Initialize(bootInfo->framebuffer->bufferBase, bootInfo->framebuffer->bufferSize, bootInfo->framebuffer->width,
						bootInfo->framebuffer->height, bootInfo->framebuffer->pixelsPerScanline);
	m_renderer.ClearBuffer(MAKE_COLOR(25, 25, 25, 255));

	m_tty.Initialize(&m_renderer, bootInfo->basicFont);
	m_tty.PrintString("Test String\0", 11);
}