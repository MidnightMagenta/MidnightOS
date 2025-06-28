#include <boot/init.hpp>

void MdOS::init_krnl(BootInfo *bootInfo) {
	init_debug_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);

	DEBUG_LOG("EOF\n");
}

void MdOS::init_debug_IO(BootExtra *bootExtra) {
	g_renderer.init(bootExtra->framebuffer);
	g_renderer.clear_buffer(MdOS::defaultBgColor);
	MdOS::Teletype::init(&g_renderer, bootExtra->basicFont);
	MdOS::CharSink::register_char_sink(MdOS::IO::BasicSerial::write_serial);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	mdos_mem_load_gdt(&g_gdtDescriptor);

	MdOS::Memory::BumpAllocator::init(reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.baseAddr),
									  reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.topAddr));

	if (MdOS::Memory::PMM::init(bootInfo->map) != MdOS::Result::SUCCESS) {
		PRINT_INFO("PMM initialized with status other than Result::SUCESS");
	}
}