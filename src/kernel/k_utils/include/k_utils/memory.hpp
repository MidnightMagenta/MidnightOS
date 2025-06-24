#ifndef K_UTILS_MEMORY_H
#define K_UTILS_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <k_utils/types.h>

namespace utils {
void *memset(void *ptr, int value, size_t num);
}// namespace utils

#endif