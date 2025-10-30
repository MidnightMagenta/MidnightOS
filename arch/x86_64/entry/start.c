#include <abi/boot/boot_info.h>
#include <debug/dbg_serial.h>
#include <debug/dbgio.h>

void s0_entry(bi_bootinfo_t *bootInfo) {
  dbg_serial_init();
  dbg_register_sink(dbg_serial_putc);

  dbg_msg("EOF\n");
  __asm__ volatile("cli\n"
                   "1: hlt\n"
                   "jmp 1b\n");
}