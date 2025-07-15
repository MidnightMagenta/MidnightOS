#ifndef MDOS_LIBK_ASSERT_H
#define MDOS_LIBK_ASSERT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <IO/kprint.h>
#define kassert(condition)                                                                                             \
	if (!(condition)) {                                                                                                \
		kprint("[ASSERTION FAILED][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, #condition);            \
		while (true) { __asm__("cli; hlt"); }                                                                          \
	}

#ifdef __cplusplus
}
#endif
#endif