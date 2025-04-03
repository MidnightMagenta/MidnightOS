#include "../include/boot_info.hpp"
#include "../include/kernel.hpp"

MdOS::Kernel krnl;

extern "C" void kernel_entry(BootInfo *bootInfo) {
	krnl.run(bootInfo);
	return;
}