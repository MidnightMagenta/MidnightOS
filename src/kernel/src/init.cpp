#include "../include/init.hpp"

void MdOS::init_krnl(BootInfo *bootInfo) {
	init_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);

	MdOS::IO::kprint(
			"Bootstrap heap: %lu KiB\n\tBase - paddr: 0x%lx vaddr: 0x%lx\n\tTop  - paddr: 0x%lx vaddr: 0x%lx\n",
			bootInfo->bootstrapMem.size / 1024, bootInfo->bootstrapMem.basePaddr, bootInfo->bootstrapMem.baseAddr,
			bootInfo->bootstrapMem.topPaddr, bootInfo->bootstrapMem.topAddr);
	MdOS::IO::kprint("Framebuffer addr: 0x%lx\n", bootInfo->bootExtra.framebuffer->bufferBase);
	MdOS::IO::kprint("\nEOF\n");
}

void MdOS::init_IO(BootExtra *bootExtra) {
	g_renderer.init(bootExtra->framebuffer->bufferBase, bootExtra->framebuffer->bufferSize,
					bootExtra->framebuffer->width, bootExtra->framebuffer->height,
					bootExtra->framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(DEFAULT_CLEAR_COLOR);

	MdOS::IO::kprintSystem::init(&g_renderer, bootExtra->basicFont);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	MdOS::Memory::g_bumpAlloc.init(uintptr_t(bootInfo->bootstrapMem.baseAddr),
								   uintptr_t(bootInfo->bootstrapMem.topAddr));
	MdOS::Memory::PhysicalMemoryManager::init(bootInfo->map);
}