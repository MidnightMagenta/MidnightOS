#include "../include/boot_info.hpp"

extern "C" void kernel_entry(BootInfo* bootInfo) {
	__asm__("hlt");
	return;
}