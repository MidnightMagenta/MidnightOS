#include "../include/MidnightBoot.h"
#include "../include/k_renderer.h"
#include "../include/cstr.h"

extern "C" void _start(BootInfo_t *bootInfo) {
	asm volatile ("cli");
	k_renderer renderer(bootInfo->frameBuffer, bootInfo->initialFont,
						k_math::GetUIntColor(0, 0, 100, 255),
						k_math::GetUIntColor(255, 255, 255, 255));

	renderer.Print("Test string\n");
	renderer.Print(k_string::to_string((uint64_t)12340987));
	renderer.Print("\n");
	renderer.Print(k_string::to_string((int64_t)-12340987));
	renderer.Print("\n");
	renderer.Print(k_string::to_string(-1.18649f));
	renderer.Print("\n");
	renderer.Print(k_string::to_string((double)13.02218));
	renderer.Print("\n");
	renderer.Print(k_string::to_hstring((uint64_t)0xF0));
	renderer.Print("\n");
	renderer.Print(k_string::to_hstring((uint32_t)0xF0));
	renderer.Print("\n");
	renderer.Print(k_string::to_hstring((uint16_t)0xF0));
	renderer.Print("\n");
	renderer.Print(k_string::to_hstring((uint8_t)0xF0));
	return;
}