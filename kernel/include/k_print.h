#ifndef _K_PRINT_H
#define _K_PRINT_H

#include "MidnightBoot.h"

void initializeScreen(FrameBuffer_t* frameBuffer, PSF1_FONT* font, unsigned int clearColor, unsigned int color);
void putc(unsigned int color, char chr, unsigned int xOff, unsigned int yOff);

void SetColor(unsigned int color);
void k_print(char* str);
void ClearScreen();

#endif