#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>

namespace utils {
template<size_t bmp_size>
class FixedBitmap {
public:
	FixedBitmap();
	~FixedBitmap();

private:
	uint8_t bitmap[bmp_size];
};
}// namespace utils

#endif