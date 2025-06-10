#include "../include/init.hpp"

void MdOS::init_krnl(BootInfo *bootInfo) {
	init_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);

	MdOS::IO::kprint("\nEOF\n");
}

void MdOS::init_IO(BootExtra *bootExtra) {
	g_renderer.init(bootExtra->framebuffer->bufferBase, bootExtra->framebuffer->bufferSize,
					bootExtra->framebuffer->width, bootExtra->framebuffer->height,
					bootExtra->framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(MAKE_COLOR(0, 0, 64, 255));

	MdOS::IO::kprintSystem::init(&g_renderer, bootExtra->basicFont);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	MdOS::Memory::g_bumpAlloc.init(uintptr_t(bootInfo->bootstrapMem.baseAddr),
								   uintptr_t(bootInfo->bootstrapMem.topAddr));
	MdOS::Memory::PhysicalMemoryManager::init(bootInfo->map);
}