#include "../include/boot_info.hpp"
#include "../include/kernel.hpp"

MdOS::Kernel krnl;

extern "C" void kernel_entry(BootInfo *bootInfo) {
	krnl.run(bootInfo);
	return;
}

extern "C" void __cxa_pure_virtual() {
	// Do nothing or print an error message.
}
