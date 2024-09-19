#ifndef _K_PRINT_H
#define _K_PRINT_H

#include "../include/MidnightBoot.h"
#include "../include/math.h"

class k_renderer {
public:
	k_renderer(FrameBuffer_t *frameBuffer, PSF1_FONT *font,
			   unsigned int clearColor, unsigned int color);
	void putc(unsigned int color, char chr, unsigned int xOff,
			  unsigned int yOff);
	void SetColor(unsigned int color) { color = color; }
	void SetClearColor(unsigned int color) {l_clear_color = color;}
	void Print(const char *str);
	void ClearScreen();

private:
	FrameBuffer_t *l_frameBuffer;
	PSF1_FONT *l_font;

	unsigned int l_clear_color;
	unsigned int l_color;
	k_math::point cursorPostiton;
};
#endif