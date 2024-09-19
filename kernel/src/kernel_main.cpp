#include "../include/MidnightBoot.h"
#include "../include/k_renderer.h"
#include "../include/cstr.h"

extern "C" void _start(BootInfo_t *bootInfo) {
	asm volatile ("cli");
	k_renderer renderer(bootInfo->frameBuffer, bootInfo->initialFont,
						k_math::GetUIntColor(0, 0, 100, 255),
						k_math::GetUIntColor(255, 255, 255, 255));

	uint64_t memMapEntries = bootInfo->mMapSize / bootInfo->mMapDescriptorSize;
	for(int i = 0; i < memMapEntries; i++){
		EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)bootInfo->memMap + (i * bootInfo->mMapDescriptorSize));
		renderer.Print(EFI_MEMORY_TYPE_STRINGS[descriptor->type]);
		renderer.Print(" Size: ");
		renderer.SetColor(0xFFFF0000);
		renderer.Print(k_string::to_string(descriptor->numPages * 4096 / 1024));
		renderer.Print("Kb");
		renderer.SetColor(0xFFFFFFFF);
		renderer.SetCursorPosition(0, renderer.GetCursorPosition().y + 18);
	}
	return;
}