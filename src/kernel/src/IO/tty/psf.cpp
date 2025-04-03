#include "../../../include/IO/tty/psf1.hpp"

void MdOS::PSF_DrawableFont::InitializeFont(PSF1_Font *font){
    this->glyphs = (char*) font->glyphs;
    this->glyphHeight = font->header->charSize;
    this->glyphWidth = 8;
}

void MdOS::PSF_DrawableFont::InitializeFont(PSF2_Font *font) {
    /*stub*/
}