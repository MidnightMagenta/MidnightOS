#ifndef MDOSBOOT_DEBUG_H
#define MDOSBOOT_DEBUG_H

#include <efi.h>
#include <efilib.h>

#define DO_DEBUG_MESSAGE 1
#define DO_DEBUG_WARN 1

#if DO_DEBUG_MESSAGE
#define DBG_MSG(msg, ...) Print(L##msg, ##__VA_ARGS__)
#else
#define DBG_MSG(msg, ...)
#endif

#if DO_DEBUG_WARN
#define DBG_WARN(msg, ...) Print(L##msg, ##__VA_ARGS__)
#else
#define DBG_WARN(msg, ...)
#endif

#endif