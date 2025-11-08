#ifndef _NYX_DBG_SERIAL_H
#define _NYX_DBG_SERIAL_H

#include <nyx/result.h>

nyx_result dbg_serial_init();
void         dbg_serial_putc(char c);

#endif