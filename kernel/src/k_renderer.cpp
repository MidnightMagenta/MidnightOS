#include "../include/k_renderer.h"

k_renderer::k_renderer(FrameBuffer_t *frameBuffer, PSF1_FONT *font,
					   unsigned int clearColor, unsigned int color) {
	l_frameBuffer = frameBuffer;
	l_font = font;

	l_clear_color = clearColor;
	l_color = color;
	cursorPostiton.x = 0;
	cursorPostiton.y = 0;

	ClearScreen();
}

void k_renderer::putc(unsigned int color, char chr, unsigned int xOff,
					  unsigned int yOff) {
	unsigned int *pixPtr = (unsigned int *) l_frameBuffer->base_addr;
	char *fontPtr = (char *) l_font->glyphs + (chr * l_font->header->char_size);

	for (unsigned long y = yOff; y < yOff + l_font->header->char_size; y++) {
		for (unsigned long x = xOff; x < xOff + 8; x++) {
			if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0) {
				*(unsigned int *) (pixPtr + x +
								   (y * l_frameBuffer->pixel_per_scanline)) =
						color;
			}
		}
		fontPtr++;
	}
}

void k_renderer::Print(const char *str) {
	char *chr = (char *) str;
	while (*chr != 0) {
		if (*chr == '\n') {
			cursorPostiton.x = 0;
			cursorPostiton.y += l_font->header->char_size;
			chr++;
			continue;
		}
		if (*chr == '\r') { cursorPostiton.x = 0; }
		putc(l_color, *chr, cursorPostiton.x, cursorPostiton.y);
		if ((cursorPostiton.x += 8) >= l_frameBuffer->width) {
			cursorPostiton.x = 0;
			if ((cursorPostiton.y += l_font->header->char_size) >=
				l_frameBuffer->height) {
				ClearScreen();
				cursorPostiton.y = 0;
			}
		}

		chr++;
	}
}

void k_renderer::ClearScreen() {
	for (unsigned int i = 0; i < l_frameBuffer->size; i += 4) {
		*(unsigned int *) (i + (char *) l_frameBuffer->base_addr) =
				l_clear_color;
	}
}