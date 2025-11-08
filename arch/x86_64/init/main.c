#include "asm/cpu.h"
#include <abi/boot/boot_info.h>
#include <asm/idt.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <debug/dbg_serial.h>
#include <debug/dbgio.h>

#ifdef _DEBUG
#define init_dbg_print()                                                                                               \
    dbg_serial_init();                                                                                                 \
    dbg_register_sink(dbg_serial_putc)
#else
#define init_dbg_print()
#endif

extern void dbg_start(const struct int_info *);

void main(bi_bootinfo_t *bootInfo) {
    (void) bootInfo;
    init_dbg_print();

    idt_setup_early_traps();
    traps_register_dbg_hook(dbg_start);

    __asm__ volatile("int3");

    dbg_print("EOF");

    cli();
    halt_forever();
}