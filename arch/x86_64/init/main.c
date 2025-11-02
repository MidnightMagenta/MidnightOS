#include <abi/boot/boot_info.h>
#include <asm/system.h>
#include <debug/dbg_serial.h>
#include <debug/dbgio.h>

void main(bi_bootinfo_t *bootInfo) {
    dbg_serial_init();
    dbg_register_sink(dbg_serial_putc);

    dbg_msg("EOF");

    cli();
    halt_forever();
}