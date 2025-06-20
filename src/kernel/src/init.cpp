#include "../include/init.hpp"

MdOS::Memory::PhysicalMemoryManager g_pmm;

void MdOS::init_krnl(BootInfo *bootInfo) {
	init_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);
	kprint("\nEOF\n");
}

void MdOS::init_IO(BootExtra *bootExtra) {
	g_renderer.init(bootExtra->framebuffer->bufferBase, bootExtra->framebuffer->bufferSize,
					bootExtra->framebuffer->width, bootExtra->framebuffer->height,
					bootExtra->framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(MdOS::defaultBgColor);

	MdOS::IO::kprintSystem::init(&g_renderer, bootExtra->basicFont);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	MdOS::Memory::BumpAllocator::init(uintptr_t(bootInfo->bootstrapMem.baseAddr),
									  uintptr_t(bootInfo->bootstrapMem.topAddr));
	g_pmm.init(bootInfo->map);
}