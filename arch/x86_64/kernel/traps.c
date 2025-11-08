#include <asm/cpu.h>
#include <asm/traps.h>
#include <debug/dbgio.h>

DEFINE_IRQENTRY(int3) { dbg_print("Test irq3: %d\n", info->frame.rip); }