#ifndef _DBGIO_H
#define _DBGIO_H

#include <stddef.h>

typedef void (*dbg_charsink_t)(char);

int  dbg_register_sink(dbg_charsink_t sink);
void dbg_unregister_sink(dbg_charsink_t sink);
void dbg_sinkchr(char c);

size_t dbg_msg(const char *fmt, ...);

#endif