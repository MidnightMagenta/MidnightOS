#include <boot/init.hpp>
#include <error/panic.h>
void MdOS::init_krnl(BootInfo *bootInfo) {
	init_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);
	PANIC("Test panic", TEST_ERROR);
	MdOS::IO::kprint("\nEOF\n");
}

void MdOS::init_IO(BootExtra *bootExtra) {
	g_renderer.init(bootExtra->framebuffer->bufferBase, bootExtra->framebuffer->bufferSize,
					bootExtra->framebuffer->width, bootExtra->framebuffer->height,
					bootExtra->framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(MdOS::defaultBgColor);

	MdOS::IO::kprintSystem::init(&g_renderer, bootExtra->basicFont);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	mdos_mem_load_gdt(&g_gdtDescriptor);

	MdOS::Memory::BumpAllocator::init(uintptr_t(bootInfo->bootstrapMem.baseAddr),
									  uintptr_t(bootInfo->bootstrapMem.topAddr));

	if (MdOS::Memory::PMM::init(bootInfo->map) != MdOS::Result::SUCCESS) {
		PRINT_INFO("PMM initialized with status other than Result::SUCESS");
	}
}