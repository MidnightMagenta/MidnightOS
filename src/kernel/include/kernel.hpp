#ifndef KERNEL_H
#define KERNEL_H

#include "../include/GOP_renderer.hpp"
#include "../include/boot_info.hpp"
#include "../include/tty.hpp"

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