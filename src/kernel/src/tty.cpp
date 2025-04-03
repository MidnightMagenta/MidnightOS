#include "../include/tty.hpp"

void MdOS::Teletype::putc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset) {
	//TODO: turn this into a blit
	char *glyph = m_font[chr];
	uint32_t *pixel = (uint32_t *) m_renderer->m_bufferBase;
	for (uint32_t y = yOffset; y < yOffset + m_font.glyphHeight; y++) {
		for (uint32_t x = xOffset; x < xOffset + 8; x++) {
			if ((*glyph & (0b10000000 >> (x - xOffset))) > 0) { *(uint32_t *) (pixel + x + (y * m_renderer->m_pixelsPerScanline)) = color; }
		}
		glyph++;
	}
}

void MdOS::Teletype::PrintString(const char *str, size_t strlen) {
	for (size_t i = 0; i < strlen; i++) {
		putc(str[i], m_color, m_xOffset, m_yOffset);
		m_xOffset += (uint32_t) m_font.glyphWidth;
		if (m_xOffset >= m_renderer->m_width) {
			m_yOffset += (uint32_t) m_font.glyphHeight;
			m_xOffset = 0;
		}
	}
}