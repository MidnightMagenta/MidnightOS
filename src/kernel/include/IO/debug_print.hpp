#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include "../../include/IO/kprint.hpp"

#define VERBOSE_FUNCTION_NAMES false

#if VERBOSE_FUNCTION_NAMES
#define PRINT_ERROR(msg) kprint("[ERROR][%s] %s\n", __PRETTY_FUNCTION__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][%s] %s\n", __PRETTY_FUNCTION__, msg)
#define PRINT_INFO(msg) kprint("[INFO][%s] %s\n", __PRETTY_FUNCTION__, msg)
#else
#define PRINT_ERROR(msg) kprint("[ERROR][Function: %s] %s\n", __func__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][Function: %s] %s\n", __func__, msg)
#define PRINT_INFO(msg) kprint("[INFO][Function: %s] %s\n", __func__, msg)
#endif

#endif