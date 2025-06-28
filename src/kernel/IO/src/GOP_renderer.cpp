#include <IO/GOP_renderer.hpp>

void MdOS::GOP_Renderer::clear_buffer(uint32_t clearColor) {
	if (!m_initialized) { return; }
	for (uint32_t i = 0; i < m_bufferSize; i += 4) { *(uint32_t *) (i + (char *) m_bufferBase) = clearColor; }
}

void MdOS::GOP_Renderer::init(void *bufferBase, uint64_t bufferSize, uint32_t width, uint32_t height, uint32_t ppsl) {
	if (bufferBase != nullptr && bufferSize > 0) {
		m_initialized = true;
	} else {
		return;
	}
	m_bufferBase = bufferBase;
	m_bufferSize = bufferSize;
	m_width = width;
	m_height = height;
	m_pixelsPerScanline = ppsl;
}
