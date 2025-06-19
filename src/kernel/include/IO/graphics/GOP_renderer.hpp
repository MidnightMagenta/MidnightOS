#ifndef GOP_RENDERER_H
#define GOP_RENDERER_H

#include "../../../include/IO/tty/psf.hpp"
#include <stdint.h>

namespace MdOS {
class GOP_Renderer {
public:
	GOP_Renderer() {}
	GOP_Renderer(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl) {
		init(bufferBase, bufferSize, width, height, ppsl);
	}
	~GOP_Renderer() {}

	static constexpr uint32_t make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		return uint32_t((uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b));
	}
	static constexpr uint32_t make_color_hex(uint32_t hex, uint8_t a) { return uint32_t((uint32_t(a) << 24) | hex); }

	void init(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl);
	void clear_buffer(uint32_t clearColor);

	inline void *framebuffer_base() { return m_bufferBase; }
	inline uint64_t framebuffer_size() { return m_bufferSize; }
	inline uint32_t framebuffer_width() { return m_width; }
	inline uint32_t framebuffer_height() { return m_height; }
	inline uint32_t framebuffer_pps() { return m_pixelsPerScanline; }
private:
	void *m_bufferBase;
	uint64_t m_bufferSize;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pixelsPerScanline;
};

static constexpr uint32_t defaultBgColor = GOP_Renderer::make_color(0, 0, 64, 255);
static GOP_Renderer g_renderer;

}// namespace MdOS

#endif