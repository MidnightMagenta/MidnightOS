#include "../include/init.hpp"

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
	MdOS::Memory::PMM::init(bootInfo->map);

	kprint("Max avail mem: %lu KiB\nUnusable memory: %lu KiB\nFree mem: %lu KiB\nUsed mem: %lu KiB\nReserved "
		   "mem: %lu KiB\n\n",
		   MdOS::Memory::PMM::max_mem_size() / 1024, MdOS::Memory::PMM::unusable_mem_size() / 1024, MdOS::Memory::PMM::free_mem_size() / 1024,
		   MdOS::Memory::PMM::used_mem_size() / 1024, MdOS::Memory::PMM::reserved_mem_size() / 1024);
	MdOS::Memory::PMM::PhysicalMemoryAllocation testAllocs[10];
	for (size_t i = 0; i < 10; i++) {
		MdOS::Memory::PMM::alloc_pages(i + 1, &testAllocs[i]);
		kprint("Allocated address: 0x%lx | Page count: %lu\n", testAllocs[i].base, testAllocs[i].numPages);
	}
	kprint("Max avail mem: %lu KiB\nUnusable memory: %lu KiB\nFree mem: %lu KiB\nUsed mem: %lu KiB\nReserved "
		   "mem: %lu KiB\n\n",
		   MdOS::Memory::PMM::max_mem_size() / 1024, MdOS::Memory::PMM::unusable_mem_size() / 1024, MdOS::Memory::PMM::free_mem_size() / 1024,
		   MdOS::Memory::PMM::used_mem_size() / 1024, MdOS::Memory::PMM::reserved_mem_size() / 1024);
	for (size_t i = 0; i < 10; i++) { MdOS::Memory::PMM::free_pages(testAllocs[i]); }
	kprint("Max avail mem: %lu KiB\nUnusable memory: %lu KiB\nFree mem: %lu KiB\nUsed mem: %lu KiB\nReserved "
		   "mem: %lu KiB\n\n",
		   MdOS::Memory::PMM::max_mem_size() / 1024, MdOS::Memory::PMM::unusable_mem_size() / 1024, MdOS::Memory::PMM::free_mem_size() / 1024,
		   MdOS::Memory::PMM::used_mem_size() / 1024, MdOS::Memory::PMM::reserved_mem_size() / 1024);
}