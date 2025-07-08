#include <IO/tty.hpp>

void MdOS::teletype::init(GOP_Renderer *renderer, PSF1_Font *font) {
	if (renderer != nullptr && font != nullptr) {
		m_renderer = renderer;
		m_font.init(font);
		m_graphicsAvail = true;
		MdOS::CharSink::register_char_sink(MdOS::teletype::putc);
	}
}

void MdOS::teletype::printc(const char chr, uint32_t color, uint32_t xOffset, uint32_t yOffset) {
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

void MdOS::teletype::putc(char c) {
	if (m_graphicsAvail) {
		if (c == '\n') {
			m_yOffset += uint32_t(m_font.glyphHeight);
			m_xOffset = 0;
		} else if (c == '\r') {
			m_xOffset = 0;
		} else if (c == '\t') {
			m_xOffset += m_font.glyphWidth * 3;
		} else if (c == '\t') {
			if (int32_t((int32_t(m_xOffset) - m_font.glyphWidth) < 0)) {
				m_xOffset = m_renderer->framebuffer_width() - m_font.glyphWidth;
				m_yOffset -= m_font.glyphHeight;
			} else {
				m_xOffset -= m_font.glyphWidth;
			}
		} else if (c == '\'') {
			printc('\'', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else if (c == '\"') {
			printc('\"', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else if (c == '\\') {
			printc('\\', m_color, m_xOffset, m_yOffset);
			m_xOffset += uint32_t(m_font.glyphWidth);
		} else {
			printc(c, m_color, m_xOffset, m_yOffset);
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