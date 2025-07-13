#include <IO/GOP_renderer.hpp>
#include <IO/debug_print.h>
#include <IO/kprint.hpp>
#include <boot/init.hpp>
#include <boot/kernel_status.h>
#include <error/panic.h>
#include <klibc/stdlib.h>
#include <memory/allocators/bump_allocator.hpp>
#include <memory/gdt.h>
#include <memory/new.hpp>
#include <memory/paging.hpp>
#include <memory/pmm.hpp>
#include <stdint.h>

void MdOS::init_krnl(BootInfo *bootInfo) {
	PROFILE_SCOPE("init_krnl");
	init_debug_IO(&bootInfo->bootExtra);
	init_memory(bootInfo);

	MdOS_krnlStatus_kernelReady = true;
	MdOS_krnlStatus_globalObjectsReady = true;

	MdOS::mem::phys::print_mem_map();
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

	// initialize the GDT
	GDTDescriptor *dsc = &g_gdtDescriptor;
	mdos_mem_load_gdt(dsc);

	// initialize the early bump allocator
	MdOS::mem::g_bumpAlloc = new (bootInfo->bootstrapMem.baseAddr) MdOS::mem::BumpAllocator();
	MdOS::mem::g_bumpAlloc->init(reinterpret_cast<uintptr_t>((void *) (uintptr_t(bootInfo->bootstrapMem.baseAddr) +
																	   sizeof(MdOS::mem::BumpAllocator))),
								 reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.topAddr));

	// initialize the physical memory manager
	if (MdOS::mem::phys::init(bootInfo->map, bootInfo->kernelSections, bootInfo->kernelSectionCount) != MDOS_SUCCESS) {
		DEBUG_LOG("PMM initialized sucessfully with status other than Result::SUCESS\n");
	}

	// initialize the default virtual memory manager
	MdOS::mem::virt::g_defaultVMM =
			new (malloc(sizeof(MdOS::mem::virt::VirtualMemoryManagerPML4))) MdOS::mem::virt::VirtualMemoryManagerPML4();

	if (MdOS::mem::virt::g_defaultVMM->init() != MDOS_SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}

	if (MdOS::mem::virt::map_kernel(bootInfo->kernelSections, bootInfo->kernelSectionCount, bootInfo->map,
									bootInfo->bootstrapMem, bootInfo->bootExtra.framebuffer,
									MdOS::mem::virt::g_defaultVMM) != MDOS_SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}
	MdOS::mem::virt::VirtualMemoryManagerPML4::bind_vmm(MdOS::mem::virt::g_defaultVMM);
	MdOS::mem::virt::VirtualMemoryManagerPML4::get_bound_vmm()->activate();
}