#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <IO/kprint.h>

#define VERBOSE_FUNCTION_NAMES false

#if VERBOSE_FUNCTION_NAMES
#define PRINT_ERROR(msg) kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define PRINT_INFO(msg) kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#else
#define PRINT_ERROR(msg) kprint("[ERROR][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_WARNING(msg) kprint("[WARNING][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#define PRINT_INFO(msg) kprint("[INFO][%s:%d in function: %s] %s\n", __FILE__, __LINE__, __func__, msg)
#endif

#ifdef _DEBUG
#define DEBUG_LOG(msg, ...) kprint("[DEBUG] " msg, ##__VA_ARGS__)
#else
#define DEBUG_LOG(msg, ...) /*void*/
#endif

#ifdef _LOG_ALLOCATIONS
#define ALLOC_LOG(msg, ...) kprint("[ALLOCATION: %s] " msg "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#define ALLOC_LOG(msg, ...) /*void*/
#endif

#ifdef __cplusplus
}
#endif
#endif