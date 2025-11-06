#ifndef NYXBOOT_UTILS_H
#define NYXBOOT_UTILS_H

#include "../include/debug.h"

#define ALIGN_UP(v, a, t)   ((t) v + ((t) a - 1)) & (~((t) a - 1))
#define ALIGN_DOWN(v, a, t) ((t) v & ~((t) a - 1))

#define EFI_TRY_RET(expr)                                                                                              \
    do {                                                                                                               \
        EFI_STATUS _s = (expr);                                                                                        \
        if (EFI_ERROR(_s)) { return _s; }                                                                              \
    } while (0)

#define EFI_TRY(expr, label, msg)                                                                                      \
    do {                                                                                                               \
        EFI_STATUS _s = (expr);                                                                                        \
        if (EFI_ERROR(_s)) {                                                                                           \
            DBG_WARN(msg);                                                                                             \
            res = _s;                                                                                                  \
            goto label;                                                                                                \
        }                                                                                                              \
    } while (0)

#endif