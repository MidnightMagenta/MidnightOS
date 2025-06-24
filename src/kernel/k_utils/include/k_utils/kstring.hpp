#ifndef MDOS_K_UTILS_KSTRING_H
#define MDOS_K_UTILS_KSTRING_H

#include <stddef.h>
#include <stdint.h>

namespace MdOS::string {
size_t strlen(const char *str);

const char *to_string(uint64_t num);
inline const char *to_string(uint32_t num) { return to_string(uint64_t(num)); }
inline const char *to_string(uint16_t num) { return to_string(uint64_t(num)); }
inline const char *to_string(uint8_t num) { return to_string(uint64_t(num)); }

const char *to_hstring(uint64_t num);
const char *to_hstring(uint32_t num);
const char *to_hstring(uint16_t num);
const char *to_hstring(uint8_t num);

const char *to_string(int64_t num);
inline const char *to_string(int32_t num) { return to_string(int64_t(num)); }
inline const char *to_string(int16_t num) { return to_string(int64_t(num)); }
inline const char *to_string(int8_t num) { return to_string(int64_t(num)); }

const char *to_string(float num, unsigned int decimal_places);
const char *to_string(double num, unsigned int decimal_places);
inline const char *to_string(float num) { return to_string(num, 4); }
inline const char *to_string(double num) { return to_string(num, 4); }
}// namespace MdOS::string


#endif