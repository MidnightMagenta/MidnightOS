#ifndef KERNEL_H
#define KERNEL_H

#include "../include/IO/graphics/GOP_renderer.hpp"
#include "../include/IO/tty/tty.hpp"
#include "../include/boot_info.hpp"

namespace MdOS {
class Kernel {
public:
	void run(BootInfo *bootInfo);

private:
	GOP_Renderer m_renderer;
	Teletype m_tty;
};
}// namespace MdOS
#endif