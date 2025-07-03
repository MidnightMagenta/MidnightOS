#ifndef MDOS_KERNEL_H
#define MDOS_KERNEL_H

#include <IO/GOP_renderer.hpp>
#include <IO/debug_print.h>
#include <IO/kprint.hpp>
#include <boot/boot_info.hpp>
#include <error/panic.h>
#include <memory/gdt.h>
#include <memory/paging.hpp>
#include <memory/pmm.hpp>
#include <stdint.h>

namespace MdOS {
void init_krnl(BootInfo *bootInfo);
void init_debug_IO(BootExtra *bootExtra);
void init_memory(BootInfo *bootInfo);
}// namespace MdOS
#endif