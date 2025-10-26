#include "../debug/dbg_serial.h"
#include <debug/dbgio.h>

void s0_entry() {
	dbg_serial_init();
	dbg_register_sink(dbg_serial_putc);

	dbg_msg("Hello kernel!\n");

	return;
}