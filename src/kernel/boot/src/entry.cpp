#include "../include/boot_info.hpp"
#include "../include/init.hpp"

extern "C" void kernel_entry(BootInfo *bootInfo) {
	MdOS::init_krnl(bootInfo);
	return;
}