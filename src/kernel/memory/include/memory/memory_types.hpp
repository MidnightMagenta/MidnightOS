#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H

#include <stddef.h>
#include <stdint.h>

namespace MdOS::Memory {
using PhysicalAddress = uintptr_t;
using VirtualAddress = uintptr_t;
using MemSize = size_t;
}// namespace MdOS::Memory


#endif