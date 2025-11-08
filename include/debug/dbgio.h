#ifndef _NYX_DBGIO_H
#define _NYX_DBGIO_H

#include <nyx/result.h>
#include <stddef.h>

typedef void (*dbg_charsink_t)(char);

nyx_result dbg_register_sink(dbg_charsink_t sink);
void         dbg_unregister_sink(dbg_charsink_t sink);
void         dbg_sinkchr(char c);

size_t dbg_msg(const char *fmt, ...);

#ifdef _DEBUG
#define dbg_print(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#else
#define dbg_print(fmt, ...)
#endif
#endif