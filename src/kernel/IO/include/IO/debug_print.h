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

#if defined(_DEBUG) && defined(_LOG_VERBOSITY)

#if _LOG_VERBOSITY >= 1
#define DEBUG_LOG_VB1(msg, ...) kprint("[DEBUG] " msg, ##__VA_ARGS__)
#else
#define DEBUG_LOG_VB1(msg, ...) /*void*/
#endif

#if _LOG_VERBOSITY >= 2
#define DEBUG_LOG_VB2(msg, ...) kprint("[DEBUG VERBOSE=2] " msg, ##__VA_ARGS__)
#else
#define DEBUG_LOG_VB2(msg, ...) /*void*/
#endif

#if _LOG_VERBOSITY >= 3
#define DEBUG_LOG_VB3(msg, ...) kprint("[DEBUG VERBOSE=3] " msg, ##__VA_ARGS__)
#define ALLOC_LOG(msg, ...) kprint("[ALLOCATION: %s] " msg "\n", __func__, ##__VA_ARGS__)
#else
#define DEBUG_LOG_VB3(msg, ...)	  /*void*/
#define ALLOC_LOG(msg, ...)		  /*void*/
#endif

#if _LOG_VERBOSITY >= 4
#define LOG_FUNC_ENTRY kprint("[DEBUG VERBOSE=3] Entered %s:%d\n", __PRETTY_FUNCTION__, __LINE__)
#define LOG_FUNC_PARAMS(msg, ...) kprint("[DEBUG VERBOSE=3] In function %s:%d" msg, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_FUNC_EXIT kprint("[DEBUG VERBOSE=3] Exiting %s:%d\n", __PRETTY_FUNCTION__, __LINE__)
#else
#define LOG_FUNC_ENTRY			  /*void*/
#define LOG_FUNC_PARAMS(msg, ...) /*void*/
#define LOG_FUNC_EXIT			  /*void*/
#endif

#else
#define DEBUG_LOG_VB1(msg, ...) /*void*/
#define DEBUG_LOG_VB2(msg, ...) /*void*/
#define DEBUG_LOG_VB3(msg, ...) /*void*/
#define ALLOC_LOG(msg, ...)		/*void*/
#endif

#ifdef __cplusplus
}
#endif
#endif