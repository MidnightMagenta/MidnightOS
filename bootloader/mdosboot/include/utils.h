#ifndef MDOSBOOT_UTILS_H
#define MDOSBOOT_UTILS_H

#include "../include/debug.h"

#define ALIGN_UP(val, alignment, castType) ((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))
#define ALIGN_DOWN(val, alignment, castType) ((castType) val & ~((castType) alignment - 1))

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