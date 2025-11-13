#include <abi/boot/boot_info.h>
#include <asm/idt.h>
#include <asm/system.h>
#include <nyx/compiler.h>
#include <debug/debug.h>

void main(bi_bootinfo_t __unused *bootInfo) {
#ifdef _DEBUG
    init_dbg_print();
#endif
    idt_setup_early_traps();

#if defined(_DEBUG) && defined(_DEBUGER_START)
    breakpoint();
#endif

    dbg_print("EOF");

    _exit(0);
}