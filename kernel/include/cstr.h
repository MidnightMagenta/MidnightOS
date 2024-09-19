#ifndef _K_STR_H
#define _K_STR_H

#include <stddef.h>
#include <stdint.h>

namespace k_string{
	size_t strlen(char* str);
	const char* to_string(uint64_t num);
	const char* to_string(int64_t num);
	const char* to_hstring(uint64_t num);
	const char* to_hstring(uint32_t num);
	const char* to_hstring(uint16_t num);
	const char* to_hstring(uint8_t num);
	const char* to_string(float num, uint8_t decimal_places);
	const char* to_string(double num, uint8_t decimal_places);

	const char* to_string(float num);
	const char* to_string(double num);
}

#endif