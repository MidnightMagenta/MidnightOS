#ifndef PSF1_H
#define PSF1_H

#include <stdint.h>

struct PSF1_Header {
	uint16_t magic;
	uint8_t fontMode;
	uint8_t charSize;
};

struct PSF2_Header {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t glyphCount;
	uint32_t bytesPerGlyph;
	uint32_t height;
	uint32_t width;
};

struct PSF1_Font {
	PSF1_Header *header;
	void *glyphs;
};

#endif