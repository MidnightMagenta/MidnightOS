#include <IO/GOP_renderer.hpp>
#include <IO/debug_print.h>
#include <IO/kprint.hpp>
#include <boot/init.hpp>
#include <boot/kernel_status.h>
#include <error/panic.h>
#include <libk/stdlib.h>
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

	DEBUG_LOG_VB1("Log verbosity: %d\n", _LOG_VERBOSITY);
	DEBUG_LOG_VB1("EOF: init_krnl\n");

	LOG_FUNC_EXIT;
}

void MdOS::init_debug_IO(BootExtra *bootExtra) {
	PROFILE_SCOPE("init_debug_IO");
	g_renderer.init(bootExtra->framebuffer);
	g_renderer.clear_buffer(MdOS::defaultBgColor);
	MdOS::teletype::init(&g_renderer, bootExtra->basicFont);
	MdOS::CharSink::register_char_sink(MdOS::IO::BasicSerial::write_serial);
	DEBUG_LOG_VB2("Debug IO available\n");
}

void MdOS::init_memory(BootInfo *bootInfo) {
	LOG_FUNC_ENTRY;
	PROFILE_SCOPE("init_memory");

	// initialize the GDT
	GDTDescriptor *dsc = &g_gdtDescriptor;
	mdos_mem_load_gdt(dsc);
	DEBUG_LOG_VB3("Initialized GDT with 0x%lx\n", dsc);

	// initialize the early bump allocator
	DEBUG_LOG_VB3("Initializing bump heap\n");
	MdOS::mem::g_bumpAlloc = new (bootInfo->bootstrapMem.baseAddr) MdOS::mem::BumpAllocator();
	DEBUG_LOG_VB3("Built MdOS::mem::BumpAllocator at 0x%lx\n", MdOS::mem::g_bumpAlloc);
	MdOS::mem::g_bumpAlloc->init(reinterpret_cast<uintptr_t>((void *) (uintptr_t(bootInfo->bootstrapMem.baseAddr) +
																	   sizeof(MdOS::mem::BumpAllocator))),
								 reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.topAddr));
	DEBUG_LOG_VB2("Bump heap ready\n");

	// initialize the physical memory manager
	DEBUG_LOG_VB3("Initializng physical memory\n");
	if (MdOS::mem::phys::init(bootInfo->map, bootInfo->kernelSections, bootInfo->kernelSectionCount) != MDOS_SUCCESS) {
		DEBUG_LOG_VB1("PMM initialized sucessfully with status other than Result::SUCESS\n");
	}
	DEBUG_LOG_VB2("Physical memory ready\n");

	// initialize the default virtual memory manager
	DEBUG_LOG_VB3("Initializng virtual memory\n");
	MdOS::mem::virt::g_krnlVMM =
			new (malloc(sizeof(MdOS::mem::virt::VirtualMemoryManagerPML4))) MdOS::mem::virt::VirtualMemoryManagerPML4();

	DEBUG_LOG_VB3("Built VMM at 0x%lx\n", MdOS::mem::virt::g_krnlVMM);

	if (MdOS::mem::virt::g_krnlVMM->init() != MDOS_SUCCESS) {
		PANIC("Failed to initialize virtual memory", MDOS_PANIC_INIT_FAIL);
	}

	DEBUG_LOG_VB3("VMM initialized\n");

	if (MdOS::mem::virt::map_kernel(bootInfo->kernelSections, bootInfo->kernelSectionCount, bootInfo->map,
									bootInfo->bootstrapMem, bootInfo->bootExtra.framebuffer,
									MdOS::mem::virt::g_krnlVMM) != MDOS_SUCCESS) {
		PANIC("Failed to initialize virtual memory", MDOS_PANIC_INIT_FAIL);
	}
	DEBUG_LOG_VB3("Virtual memory mapping complete\n");

	MdOS::mem::virt::g_krnlVMM->activate();
	DEBUG_LOG_VB2("Virtual memory ready\n");

	LOG_FUNC_EXIT;
}