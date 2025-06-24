#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <IO/kprint.hpp>

#define VERBOSE_FUNCTION_NAMES false

#if VERBOSE_FUNCTION_NAMES
#define PRINT_ERROR(msg) MdOS::IO::kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_WARNING(msg) MdOS::IO::kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_INFO(msg) MdOS::IO::kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#else
#define PRINT_ERROR(msg) MdOS::IO::kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_WARNING(msg) MdOS::IO::kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_INFO(msg) MdOS::IO::kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#endif

#define kassert(condition)                                                                                             \
	if (!(condition)) {                                                                                                \
		MdOS::IO::kprint("[ASSERTION FAILED][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, #condition);  \
		while (true) { __asm__("cli; hlt"); }                                                                          \
	}

#endif