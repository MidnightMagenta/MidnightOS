#include "../include/boot_info.hpp"
#include "../include/kernel.hpp"

extern "C" void _init();
extern "C" void _fini();

MdOS::Kernel krnl;

extern "C" void kernel_entry(BootInfo *bootInfo) {
	__asm__("cli");
	_init();
	krnl.run(bootInfo);
	__asm__("hlt");
	_fini();
	return;
}

extern "C" void __cxa_pure_virtual() {
	// Do nothing or print an error message.
}
