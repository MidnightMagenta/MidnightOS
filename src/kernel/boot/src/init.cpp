#include <IO/GOP_renderer.hpp>
#include <IO/debug_print.h>
#include <IO/kprint.hpp>
#include <boot/init.hpp>
#include <boot/kernel_status.h>
#include <error/panic.h>
#include <k_utils/utils.hpp>
#include <memory/gdt.h>
#include <memory/new.hpp>
#include <memory/paging.hpp>
#include <memory/pmm.hpp>
#include <stdint.h>
#include <memory/physical_mem_map.hpp>

using namespace MdOS::memory;
using MdOS::memory::allocators::g_bumpAlloc;

void MdOS::init_krnl(BootInfo *bootInfo) {
	PROFILE_SCOPE("init_krnl");
	init_debug_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);

	MdOS_krnlStatus_kernelReady = true;
	MdOS_krnlStatus_globalObjectsReady = true;

	MdOS::memory::PMM::print_mem_map();

	DEBUG_LOG("EOF: init_krnl\n");
}

void MdOS::init_debug_IO(BootExtra *bootExtra) {
	PROFILE_SCOPE("init_debug_IO");
	g_renderer.init(bootExtra->framebuffer);
	g_renderer.clear_buffer(MdOS::defaultBgColor);
	MdOS::teletype::init(&g_renderer, bootExtra->basicFont);
	MdOS::CharSink::register_char_sink(MdOS::IO::BasicSerial::write_serial);
}

void MdOS::init_memory(BootInfo *bootInfo) {
	PROFILE_SCOPE("init_memory");
	GDTDescriptor *dsc = &g_gdtDescriptor;
	mdos_mem_load_gdt(dsc);

	g_bumpAlloc = new (bootInfo->bootstrapMem.baseAddr) allocators::BumpAllocator();

	g_bumpAlloc->init(reinterpret_cast<uintptr_t>((void *) (uintptr_t(bootInfo->bootstrapMem.baseAddr) +
															sizeof(allocators::BumpAllocator))),
					  reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.topAddr));

	if (PMM::init(bootInfo->map) != MdOS::Result::SUCCESS) {
		DEBUG_LOG("PMM initialized sucessfully with status other than Result::SUCESS\n");
	}

	paging::g_defaultVMM =
			new (g_bumpAlloc->alloc(sizeof(paging::VirtualMemoryManagerPML4))) paging::VirtualMemoryManagerPML4();

	if (paging::g_defaultVMM->init() != MdOS::Result::SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}

	if (paging::map_kernel(bootInfo->kernelSections, bootInfo->kernelSectionCount, bootInfo->map,
						   bootInfo->bootstrapMem, bootInfo->bootExtra.framebuffer,
						   paging::g_defaultVMM) != MdOS::Result::SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}
	paging::VirtualMemoryManagerPML4::bind_vmm(paging::g_defaultVMM);
	paging::VirtualMemoryManagerPML4::get_bound_vmm()->activate();
}