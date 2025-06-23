#ifndef KERNEL_H
#define KERNEL_H

#include "../../IO/include/GOP_renderer.hpp"
#include "../../IO/include/debug_print.hpp"
#include "../../IO/include/tty.hpp"
#include "../../k_utils/include/bitmap.hpp"
#include "../../k_utils/include/kstring.hpp"
#include "../../memory/include/bump_allocator.hpp"
#include "../../memory/include/pmm.hpp"
#include "../include/boot_info.hpp"
#include <stdint.h>

namespace MdOS {
void init_krnl(BootInfo *bootInfo);
void init_IO(BootExtra *bootExtra);
void init_memory(BootInfo *bootInfo);
}// namespace MdOS
#endif