#ifndef KERNEL_H
#define KERNEL_H

#include "../include/IO/graphics/GOP_renderer.hpp"
#include "../include/IO/tty/tty.hpp"
#include "../include/boot_info.hpp"
#include "../include/memory/bump_allocator.hpp"

namespace MdOS {
class Kernel {
public:
	void run(BootInfo *bootInfo);

private:
	MdOS::Memory::BumpAllocator m_bumpAlloc;
};
}// namespace MdOS
#endif