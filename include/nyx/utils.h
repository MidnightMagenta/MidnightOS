#ifndef _NYX_UTILS_H
#define _NYX_UTILS_H


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#ifdef _DEBUG
#include <debug/debug.h>
#define BUG(msg)                                                                                                       \
    dbg_print(msg);                                                                                                    \
    breakpoint()
#else
#include <nyx/kernel.h>
#define BUG(msg) panic(msg)
#endif

#define ASSERT(cond, msg)                                                                                              \
    if (!(cond)) { BUG(msg); }

#endif