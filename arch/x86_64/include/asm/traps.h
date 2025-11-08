#ifndef _NYX_TRAPS_H
#define _NYX_TRAPS_H

void traps_register_dbg_hook(void (*hook)(const struct int_info *));

#endif