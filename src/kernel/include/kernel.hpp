#ifndef KERNEL_H
#define KERNEL_H

#include "../include/GOP_renderer.hpp"
#include "../include/boot_info.hpp"

namespace MdOS {
class Kernel {
public:
	Kernel() {}
	~Kernel() {}
	void run(BootInfo *bootInfo);

private:
	GOP_Renderer renderer;
};
}// namespace MdOS
#endif