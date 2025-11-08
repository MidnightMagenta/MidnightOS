#include <asm/cpu.h>
#include <asm/idtentry.h>
#include <debug/dbgio.h>
#include <stddef.h>

void (*dbg_hook)(const struct int_info *) = NULL;
void traps_register_dbg_hook(void (*hook)(const struct int_info *)) {
    if (hook) { dbg_hook = hook; }
}

DEFINE_IDTENTRY(int3) {
    if (dbg_hook) { dbg_hook(info); }
}