#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include "../../../include/IO/graphics/GOP_renderer.hpp"

namespace MdOS {
class Teletype {
public:
	Teletype() {}
	Teletype(GOP_Renderer *renderer, PSF1_Font *font) { Initialize(renderer, font); };
	void Initialize(GOP_Renderer *renderer, PSF1_Font *font) {
		m_renderer = renderer;
		m_font.InitializeFont(font);
	}
	~Teletype() {}

	void SetColor(uint32_t color) { m_color = color; }
    void PrintString(const char* str, size_t strlen);

private:
    void putc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset);

	uint32_t m_color = 0xFFFFFFFF;
    uint32_t m_xOffset = 0;
    uint32_t m_yOffset = 0;

	GOP_Renderer *m_renderer = nullptr;
	PSF_DrawableFont m_font;
};
}// namespace MdOS

#endif