#include <abi/boot/boot_info.h>
#include <asm/idt.h>
#include <asm/system.h>
#include <debug/debug.h>


void main(bi_bootinfo_t *bootInfo) {
    (void) bootInfo;
    init_dbg_print();

    idt_setup_early_traps();

    breakpoint();

    dbg_print("EOF");

    cli();
    halt_forever();
}