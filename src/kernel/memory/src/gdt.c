#include <memory/gdt.h>

__attribute__((aligned(0x1000))) GDT g_defaultGDT = {
		{0, 0, 0, 0x00, 0x00, 0}, {0, 0, 0, 0x9A, 0xA0, 0}, {0, 0, 0, 0x92, 0xA0, 0},
		{0, 0, 0, 0x00, 0x00, 0}, {0, 0, 0, 0xFA, 0xA0, 0}, {0, 0, 0, 0xF2, 0xA0, 0},
};

GDTDescriptor g_gdtDescriptor = {sizeof(GDT) - 1, (uint64_t)&g_defaultGDT};