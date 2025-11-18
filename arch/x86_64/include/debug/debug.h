#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef _DEBUG
#include <debug/dbg_serial.h>
#include <debug/dbgio.h>

#define init_dbg_print()                                                                                               \
    dbg_serial_init();                                                                                                 \
    dbg_register_sink(dbg_serial_putc)

#define dbg_print(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)

#else
#define init_dbg_print()
#define dbg_print(fmt, ...)
#endif

#define breakpoint() __asm__ volatile("int3")

#endif