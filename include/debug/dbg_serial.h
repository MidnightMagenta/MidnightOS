#ifndef _NYX_DBG_SERIAL_H
#define _NYX_DBG_SERIAL_H

#include <nyx/status.h>

nyx_status dbg_serial_init();
void       dbg_serial_putc(char c);
char       dbg_serial_getc();

#endif