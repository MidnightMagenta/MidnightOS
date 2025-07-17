#include <IO/GOP_renderer.hpp>
#include <stdint.h>

void MdOS::GOP_Renderer::clear_buffer(uint32_t clearColor) {
	if (!m_initialized) { return; }
	for (uint32_t i = 0; i < m_bufferSize; i += 4) { *(uint32_t *) (i + (uintptr_t) m_bufferBase) = clearColor; }
}

void MdOS::GOP_Renderer::init(GOPFramebuffer *framebuffer) {
	if (framebuffer->bufferBase != nullptr && framebuffer->bufferSize > 0) {
		m_initialized = true;
	} else {
		return;
	}
	m_bufferBase = framebuffer->bufferBase;
	m_bufferSize = framebuffer->bufferSize;
	m_width = framebuffer->width;
	m_height = framebuffer->height;
	m_pixelsPerScanline = framebuffer->pixelsPerScanline;
}
