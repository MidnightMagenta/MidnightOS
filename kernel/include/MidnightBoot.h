#ifndef _MIDNIGHT_BOOT_H
#define _MIDNIGHT_BOOT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	void *base_addr;
	size_t size;
	unsigned int width;
	unsigned int height;
	unsigned int pixel_per_scanline;
} FrameBuffer_t;

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char char_size;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER *header;
	void *glyphs;
} PSF1_FONT;

typedef uint64_t UINTN;

typedef struct {
    uint32_t type;
    void* physAddr;
    void* virtAddr; 
    uint64_t numPages;
    uint64_t attribs;
} EFI_MEMORY_DESCRIPTOR;

extern const char* EFI_MEMORY_TYPE_STRINGS[];

typedef struct {
	EFI_MEMORY_DESCRIPTOR *memMap;
	UINTN mMapSize;
	UINTN mMapDescriptorSize;

	FrameBuffer_t *frameBuffer;
	PSF1_FONT *initialFont;
} BootInfo_t;


#endif