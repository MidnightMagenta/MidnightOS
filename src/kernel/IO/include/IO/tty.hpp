#ifndef TTY_H
#define TTY_H

#include <IO/GOP_renderer.hpp>
#include <IO/serial.hpp>
#include <stddef.h>

namespace MdOS::Teletype {
inline uint32_t m_color = 0xFFFFFFFF;
inline uint32_t m_xOffset = 0;
inline uint32_t m_yOffset = 0;

inline bool m_graphicsAvail = false;
inline GOP_Renderer *m_renderer = nullptr;
inline PSF_DrawableFont m_font;

inline void init(GOP_Renderer *renderer, PSF1_Font *font) {
	if (renderer != nullptr && font != nullptr) {
		m_renderer = renderer;
		m_font.init(font);
		m_graphicsAvail = true;
	}
}

inline void set_color(uint32_t color) { m_color = color; }
void putc(char c);
void printc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset);
}// namespace MdOS::Teletype

#endif