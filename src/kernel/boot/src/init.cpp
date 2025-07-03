#include <IO/GOP_renderer.hpp>
#include <IO/debug_print.h>
#include <IO/kprint.hpp>
#include <boot/init.hpp>
#include <error/panic.h>
#include <k_utils/utils.hpp>
#include <memory/gdt.h>
#include <memory/paging.hpp>
#include <memory/pmm.hpp>
#include <stdint.h>

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
	GDTDescriptor *desc = &g_gdtDescriptor;
	mdos_mem_load_gdt(desc);

	MdOS::Memory::BumpAllocator::init(reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.baseAddr),
									  reinterpret_cast<uintptr_t>(bootInfo->bootstrapMem.topAddr));

	if (MdOS::Memory::PMM::init(bootInfo->map) != MdOS::Result::SUCCESS) {
		DEBUG_LOG("PMM initialized sucessfully with status other than Result::SUCESS\n");
	}

	if (MdOS::Memory::Paging::g_defaultVMM.init() != MdOS::Result::SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}
	uint64_t start = rdtsc();
	if (MdOS::Memory::Paging::map_kernel(bootInfo->kernelSections, bootInfo->kernelSectionCount, bootInfo->map,
										 bootInfo->bootstrapMem, bootInfo->bootExtra.framebuffer,
										 &MdOS::Memory::Paging::g_defaultVMM) != MdOS::Result::SUCCESS) {
		PANIC("Failed to initialize virtual memory", INIT_FAIL);
	}
	DEBUG_LOG("Mapping took %lu cycles\n", rdtsc() - start);
	MdOS::Memory::Paging::VirtualMemoryManagerPML4::bind_vmm(&MdOS::Memory::Paging::g_defaultVMM);
	MdOS::Memory::Paging::g_defaultVMM.activate();
}