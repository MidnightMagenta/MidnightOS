#ifndef MDOS_KERNEL_H
#define MDOS_KERNEL_H

#include <boot/boot_info.hpp>

namespace MdOS {
void init_krnl(BootInfo *bootInfo);
void init_debug_IO(BootExtra *bootExtra);
void init_memory(BootInfo *bootInfo);
}// namespace MdOS
#endif