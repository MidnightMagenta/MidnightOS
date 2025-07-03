#include <IO/psf.hpp>
#include <memory/paging.hpp>

void MdOS::PSF_DrawableFont::init(PSF1_Font *font) {
	this->glyphs = (char *) MDOS_PHYS_TO_VIRT(uintptr_t(font->glyphs));
	this->glyphHeight = font->header->charSize;
	this->glyphWidth = 8;
}

void MdOS::PSF_DrawableFont::init(PSF2_Font *font __attribute__((unused))) { /*stub*/ }