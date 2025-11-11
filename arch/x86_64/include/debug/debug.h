#ifndef _NYX_DEBUG_H
#define _NYX_DEBUG_H

#include <debug/dbg_serial.h>
#include <debug/dbgio.h>

#ifdef _DEBUG
#define init_dbg_print()                                                                                               \
    dbg_serial_init();                                                                                                 \
    dbg_register_sink(dbg_serial_putc)
#else
#define init_dbg_print()
#endif


#define breakpoint() __asm__ volatile("int3")

#endif