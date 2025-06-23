#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <IO/kprint.hpp>

#define VERBOSE_FUNCTION_NAMES false

#if VERBOSE_FUNCTION_NAMES
#define PRINT_ERROR(msg) kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_INFO(msg) kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#else
#define PRINT_ERROR(msg) kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_INFO(msg) kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#endif

#define kassert(condition)                                                                                             \
	if (!(condition)) {                                                                                                \
		kprint("[ASSERTION FAILED][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, #condition);            \
		while (true) { __asm__("cli; hlt"); }                                                                          \
	}

#endif