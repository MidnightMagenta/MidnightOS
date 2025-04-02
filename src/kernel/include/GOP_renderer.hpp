#ifndef GOP_RENDERER_H
#define GOP_RENDERER_H

#include <stdint.h>

#define MAKE_COLOR(r, g, b, a) (a << 24) | (r << 16) | (g << 8) | (b)
#define FB_PIXEL_ADDR(x, y, ppsl, fbBase) (uint32_t *) ((x + (y * ppsl) * 4) + (char *) fbBase)

namespace MdOS {
class GOP_Renderer {
public:
	GOP_Renderer() {}
	GOP_Renderer(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl);
	~GOP_Renderer() {}

	void Initialize(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl) {
		m_bufferBase = bufferBase;
		m_bufferSize = bufferSize;
		m_width = width;
		m_height = height;
		m_pixelsPerScanline = ppsl;
	}

	void ClearBuffer(uint32_t clearColor);

private:
	void *m_bufferBase;
	uint64_t m_bufferSize;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pixelsPerScanline;
};
}// namespace MdOS

#endif