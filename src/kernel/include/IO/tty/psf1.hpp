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

struct PSF2_Font{
	PSF2_Header *header;
}; //stub

namespace MdOS {
typedef char *PSF1_glyph;

struct PSF_DrawableFont {
	void InitializeFont(PSF1_Font *font);
	void InitializeFont(PSF2_Font *font);
	char *operator[](const char val) {
		if (val > 0) { return (char *) glyphs + (val * glyphHeight); }
		return nullptr;
	}

	PSF1_glyph glyphs;
	uint8_t glyphHeight;
	uint8_t glyphWidth;
};
}// namespace MdOS

#endif