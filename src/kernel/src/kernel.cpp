#include "../include/kernel.hpp"
#include "../include/IO/kprint.hpp"
#include "../include/bitmap.hpp"
#include "../include/kstring.hpp"
#include <stdint.h>

void MdOS::Kernel::run(BootInfo *bootInfo) {
	g_renderer.init(bootInfo->bootExtra.framebuffer->bufferBase, bootInfo->bootExtra.framebuffer->bufferSize,
					bootInfo->bootExtra.framebuffer->width, bootInfo->bootExtra.framebuffer->height,
					bootInfo->bootExtra.framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(MAKE_COLOR(25, 25, 25, 255));
	MdOS::IO::kprintSystem::init(&g_renderer, bootInfo->bootExtra.basicFont);

	MdOS::IO::kprint("Bootstrap heap vbase: 0x%lx\nBootstrap heap vtop:  0x%lx\n", bootInfo->bootstrapMem.baseAddr,
					 bootInfo->bootstrapMem.topAddr);
	MdOS::IO::kprint("Bootstrap heap pbase: 0x%lx\nBootstrap heap ptop:  0x%lx\n", bootInfo->bootstrapMem.basePaddr,
					 bootInfo->bootstrapMem.topPaddr);

	MdOS::Memory::g_bumpAlloc.init(uintptr_t(bootInfo->bootstrapMem.baseAddr),
								   uintptr_t(bootInfo->bootstrapMem.topAddr));

	MdOS::IO::kprint("\nEOF\n");
}