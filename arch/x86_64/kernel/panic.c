#include <asm/system.h>
#include <debug/dbgio.h>
#include <nyx/kernel.h>

void __attribute__((noreturn)) panic(const char *msg) {
    dbg_print("Kernel panic: %s\n", msg);
    cli();
    halt_forever();
}