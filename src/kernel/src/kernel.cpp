#include "../include/kernel.hpp"
#include "../include/IO/kprint.hpp"
#include "../include/kstring.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	g_renderer.Initialize(bootInfo->bootExtra.framebuffer->bufferBase, bootInfo->bootExtra.framebuffer->bufferSize,
						  bootInfo->bootExtra.framebuffer->width, bootInfo->bootExtra.framebuffer->height,
						  bootInfo->bootExtra.framebuffer->pixelsPerScanline);
	g_renderer.ClearBuffer(MAKE_COLOR(25, 25, 25, 255));
	MdOS::IO::kprintSystem::Initialize(&g_renderer, bootInfo->bootExtra.basicFont);

	MdOS::IO::kprint("Bootstrap heap base:  0x%lx\nBootstrap heap top:   0x%lx\n", bootInfo->bootstrapMem.baseAddr, bootInfo->bootstrapMem.topAddr);
	MdOS::IO::kprint("Bootstrap heap pbase: 0x%lx\nBootstrap heap ptop:  0x%lx\n", bootInfo->bootstrapMem.basePaddr, bootInfo->bootstrapMem.topPaddr);

	m_bumpAlloc.Init(bootInfo->bootstrapMem.baseAddr, bootInfo->bootstrapMem.topAddr);

	MdOS::IO::kprint("\nEOF\n");
}