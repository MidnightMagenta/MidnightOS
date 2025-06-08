#include "../include/boot_info.hpp"
#include "../include/kernel.hpp"

MdOS::Kernel kernel;

extern "C" void kernel_entry(BootInfo *bootInfo) {
	kernel.run(bootInfo);
	return;
}