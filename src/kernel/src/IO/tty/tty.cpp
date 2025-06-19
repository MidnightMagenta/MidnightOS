#include "../../../include/IO/tty/tty.hpp"

void MdOS::Teletype::putc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset) {
	//TODO: turn this into a blit
	char *glyph = m_font[chr];
	uint32_t *pixel = (uint32_t *) m_renderer->framebuffer_base();
	for (uint32_t y = yOffset; y < yOffset + m_font.glyphHeight; y++) {
		for (uint32_t x = xOffset; x < xOffset + 8; x++) {
			if ((*glyph & (0b10000000 >> (x - xOffset))) > 0) {
				*(uint32_t *) (pixel + x + (y * m_renderer->framebuffer_pps())) = color;
			}
		}
		glyph++;
	}
}

void MdOS::Teletype::print_str(const char *str, size_t strlen) {
	for (size_t i = 0; i < strlen; i++) {
		if (str[i] == '\n') {
			m_yOffset += uint32_t(m_font.glyphHeight);
			m_xOffset = 0;
		} else if (str[i] == '\r') {
			m_xOffset = 0;
		} else if (str[i] == '\t') {
			m_xOffset += m_font.glyphWidth * 3;
		} else if (str[i] == '\t') {
			if (int32_t((int32_t(m_xOffset) - m_font.glyphWidth) < 0)) {
				m_xOffset = m_renderer->framebuffer_width() - m_font.glyphWidth;
				m_yOffset -= m_font.glyphHeight;
			} else {
				m_xOffset -= m_font.glyphWidth;
			}
		} else if (str[i] == '\'') {
			putc('\'', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else if (str[i] == '\"') {
			putc('\"', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else if (str[i] == '\\') {
			putc('\\', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else {
			putc(str[i], m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		}

		if (m_xOffset >= m_renderer->framebuffer_width()) {
			m_yOffset += uint32_t(m_font.glyphHeight);
			m_xOffset = 0;
		}
		if (m_yOffset >= m_renderer->framebuffer_height()) { 
			m_renderer->clear_buffer(MdOS::defaultBgColor);
			m_yOffset = 0; 
		}
	}
}