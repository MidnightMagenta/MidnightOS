#ifndef _NYX_DBGIO_H
#define _NYX_DBGIO_H

#include <nyx/status.h>
#include <stddef.h>

typedef void (*dbg_charsink_t)(char);

nyx_status dbg_register_sink(dbg_charsink_t sink);
void       dbg_unregister_sink(dbg_charsink_t sink);
void       dbg_sinkchr(char c);

size_t dbg_msg(const char *fmt, ...);

#endif