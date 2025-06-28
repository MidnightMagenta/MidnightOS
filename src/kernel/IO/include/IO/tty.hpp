#ifndef TTY_H
#define TTY_H

#include <IO/GOP_renderer.hpp>
#include <IO/char_sink.hpp>
#include <IO/serial.hpp>
#include <stddef.h>

namespace MdOS::Teletype {
inline uint32_t m_color = 0xFFFFFFFF;
inline uint32_t m_xOffset = 0;
inline uint32_t m_yOffset = 0;

inline bool m_graphicsAvail = false;
inline GOP_Renderer *m_renderer = nullptr;
inline PSF_DrawableFont m_font;

void init(GOP_Renderer *renderer, PSF1_Font *font);
void putc(char c);
void printc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset);
inline void set_color(uint32_t color) { m_color = color; }
}// namespace MdOS::Teletype

#endif