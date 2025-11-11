#include <asm/system.h>
#include <debug/dbgio.h>
#include <nyx/kernel.h>

void __noreturn panic(const char *msg) {
    dbg_print("Kernel panic: %s\n", msg);
    cli();
    exit();
}