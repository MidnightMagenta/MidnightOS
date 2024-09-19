#include "../include/k_print.h"

FrameBuffer_t *l_frameBuffer;
PSF1_FONT *l_font;

unsigned int l_clear_color;
unsigned int l_color;
unsigned int cursor_x;
unsigned int cursor_y;

void initializeScreen(FrameBuffer_t *frameBuffer, PSF1_FONT *font,
					  unsigned int clearColor, unsigned int color) {
	l_frameBuffer = frameBuffer;
	l_font = font;

	l_clear_color = clearColor;
	l_color = color;
	cursor_x = 0;
	cursor_y = 0;

	ClearScreen();
}

void SetColor(unsigned int color) { color = color; }

void putc(unsigned int color, char chr, unsigned int xOff, unsigned int yOff) {
	unsigned int *pixPtr = (unsigned int *) l_frameBuffer->base_addr;
	char *fontPtr = l_font->glyphs + (chr * l_font->header->char_size);

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

void k_print(char *str) {
	char *chr = str;
	while (*chr != 0) {
		if (*chr == '\n') {
			cursor_x = 0;
			cursor_y += l_font->header->char_size;
			chr++;
			continue;
		}
		if (*chr == '\r') { cursor_x = 0; }
		putc(l_color, *chr, cursor_x, cursor_y);
		if ((cursor_x += 8) >= l_frameBuffer->width) {
			cursor_x = 0;
			if((cursor_y += l_font->header->char_size)>= l_frameBuffer->height){
				ClearScreen();
				cursor_y = 0;
			}
		}

		chr++;
	}
}

void ClearScreen(){
	for(unsigned int i = 0; i < l_frameBuffer->size; i+= 4){
		*(unsigned int*)(i + l_frameBuffer->base_addr) = l_clear_color;
	}
}