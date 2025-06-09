#ifndef GOP_RENDERER_H
#define GOP_RENDERER_H

#include "../../../include/IO/tty/psf.hpp"
#include <stdint.h>

#define MAKE_COLOR(r, g, b, a) (a << 24) | (r << 16) | (g << 8) | (b)
#define MAKE_COLOR_HEX(hex, a) (a << 24) | (hex)

namespace MdOS {
class GOP_Renderer {
public:
	GOP_Renderer() {}
	GOP_Renderer(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl) {
		init(bufferBase, bufferSize, width, height, ppsl);
	}
	~GOP_Renderer() {}

	void init(void *bufferBase, uint64_t bufferSize, int32_t width, uint32_t height, uint32_t ppsl);
	void clear_buffer(uint32_t clearColor);

	void *m_bufferBase;
	uint64_t m_bufferSize;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pixelsPerScanline;
};

static GOP_Renderer g_renderer;

}// namespace MdOS

#endif