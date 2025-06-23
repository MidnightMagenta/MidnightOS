#ifndef KERNEL_H
#define KERNEL_H

#include <IO/debug_print.hpp>
#include <boot/boot_info.hpp>
#include <memory/pmm.hpp>
#include <stdint.h>

namespace MdOS {
void init_krnl(BootInfo *bootInfo);
void init_IO(BootExtra *bootExtra);
void init_memory(BootInfo *bootInfo);
}// namespace MdOS
#endif