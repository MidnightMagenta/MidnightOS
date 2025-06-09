#include "../include/kernel.hpp"
#include "../include/IO/kprint.hpp"
#include "../include/bitmap.hpp"
#include "../include/kstring.hpp"

void MdOS::Kernel::run(BootInfo *bootInfo) {
	g_renderer.init(bootInfo->bootExtra.framebuffer->bufferBase, bootInfo->bootExtra.framebuffer->bufferSize,
						  bootInfo->bootExtra.framebuffer->width, bootInfo->bootExtra.framebuffer->height,
						  bootInfo->bootExtra.framebuffer->pixelsPerScanline);
	g_renderer.clear_buffer(MAKE_COLOR(25, 25, 25, 255));
	MdOS::IO::kprintSystem::init(&g_renderer, bootInfo->bootExtra.basicFont);

	MdOS::IO::kprint("Bootstrap heap base:  0x%lx\nBootstrap heap top:   0x%lx\n", bootInfo->bootstrapMem.baseAddr,
					 bootInfo->bootstrapMem.topAddr);
	MdOS::IO::kprint("Bootstrap heap pbase: 0x%lx\nBootstrap heap ptop:  0x%lx\n", bootInfo->bootstrapMem.basePaddr,
					 bootInfo->bootstrapMem.topPaddr);

	MdOS::Memory::g_bumpAlloc.init(bootInfo->bootstrapMem.baseAddr, bootInfo->bootstrapMem.topAddr);

	utils::Bitmap<uint8_t> bmp;
	bmp.init(64);
	bmp.set_range(16, 22);
	bmp.clear_range(18, 20);
	MdOS::IO::kprint("\n");
	for (size_t i = 0; i < bmp.size(); i++) {
		MdOS::IO::kprint("%d", bmp[i] ? 1 : 0);
	}
	MdOS::IO::kprint("\nFirst set entry: %u\n", bmp.find_first_set());

	MdOS::IO::kprint("\nEOF\n");
}