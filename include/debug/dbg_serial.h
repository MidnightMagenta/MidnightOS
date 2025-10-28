#ifndef MDOS_DBG_SERIAL_H
#define MDOS_DBG_SERIAL_H

#include <result.h>

mdos_result_t dbg_serial_init();
void dbg_serial_putc(char c);

#endif