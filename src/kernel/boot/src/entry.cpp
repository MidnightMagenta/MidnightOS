#include <boot/boot_info.hpp>
#include <boot/init.hpp>

extern "C" void kernel_entry(BootInfo *bootInfo) {
	MdOS::init_krnl(bootInfo);
	return;
}