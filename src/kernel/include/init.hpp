#ifndef KERNEL_H
#define KERNEL_H

#include "../include/IO/graphics/GOP_renderer.hpp"
#include "../include/IO/kprint.hpp"
#include "../include/IO/tty/tty.hpp"
#include "../include/bitmap.hpp"
#include "../include/boot_info.hpp"
#include "../include/kstring.hpp"
#include "../include/memory/bump_allocator.hpp"
#include "../include/memory/pmm.hpp"
#include <stdint.h>

namespace MdOS {
void init_krnl(BootInfo *bootInfo);
void init_IO(BootExtra *bootExtra);
void init_memory(BootInfo *bootInfo);
}// namespace MdOS
#endif