#ifndef TTY_H
#define TTY_H

#include "../../../include/IO/graphics/GOP_renderer.hpp"
#include <stddef.h>

namespace MdOS {
class Teletype {
public:
	Teletype() {}
	Teletype(GOP_Renderer *renderer, PSF1_Font *font) { init(renderer, font); };
	void init(GOP_Renderer *renderer, PSF1_Font *font) {
		m_renderer = renderer;
		m_font.init(font);
	}
	~Teletype() {}

	void set_color(uint32_t color) { m_color = color; }
	void print_str(const char *str, size_t strlen);

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