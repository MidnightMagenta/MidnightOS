#ifndef _K_MATH_H
#define _K_MATH_H

#include <stdint.h>

namespace k_math {
struct point {
	unsigned int x;
	unsigned int y;
};

static inline unsigned int GetUIntColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	unsigned int color = (a << 24) | (r << 16) | (g << 8) | b;
	return color;
}
}// namespace k_math

#endif