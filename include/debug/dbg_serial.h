#ifndef _DBG_SERIAL_H
#define _DBG_SERIAL_H

int  dbg_serial_init();
void dbg_serial_putc(char c);
char dbg_serial_getc();

#endif