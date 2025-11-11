#ifndef MDCF_CRC32_H
#define MDCF_CRC32_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t crc32_t;

crc32_t crc32(const void *data, size_t length, crc32_t init);

#endif