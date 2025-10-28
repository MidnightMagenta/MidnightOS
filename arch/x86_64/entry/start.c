#include <debug/dbg_serial.h>
#include <abi/boot/boot_info.h>
#include <debug/dbgio.h>

void s0_entry(bi_bootinfo_t *bootInfo) {
	dbg_serial_init();
	dbg_register_sink(dbg_serial_putc);

	dbg_msg("Boot info pointer is: %lx\n", bootInfo);
	dbg_msg(" Version: %d\n Magic: %x\n pMemMap: %lx\n pKernelMap: %lx\n", bootInfo->version, bootInfo->magic,
					bootInfo->pMemMap, bootInfo->pKernelMap); 

	while (1) { __asm__ volatile("hlt"); }
}