#ifndef _MDOS_DBGIO_H
#define _MDOS_DBGIO_H

#include <result.h>
#include <stddef.h>

typedef void (*dbg_charsink_t)(char);

mdos_result_t dbg_register_sink(dbg_charsink_t sink);
void dbg_unregister_sink(dbg_charsink_t sink);
void dbg_sinkchr(char c);

size_t dbg_msg(const char *fmt, ...);

#endif