#include <asm/system.h>
#include <debug/debug.h>
#include <nyx/kernel.h>

void __noreturn panic(const char *msg) {
    dbg_print("Kernel panic: %s\n", msg);
    cli();
    _exit(0);
}