#include "../include/boot_info.hpp"
#include "../include/kernel.hpp"

extern "C" void kernel_entry(BootInfo *bootInfo) {
    __asm__("cli");
	MdOS::Kernel krnl;
	krnl.run(bootInfo);
	__asm__("hlt");
	return;
}