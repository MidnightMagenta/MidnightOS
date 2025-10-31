#include <debug/dbg_serial.h>
#include <debug/dbgio.h>
#include <abi/boot/boot_info.h>

void main(bi_bootinfo_t* bootInfo) {
  dbg_serial_init();
  dbg_register_sink(dbg_serial_putc);

  dbg_msg("EOF");

  __asm__ volatile("cli");
  while (1) { __asm__ volatile("hlt"); }
}