#include "../include/MidnightBoot.h"
#include "../include/k_print.h"

void _start(BootInfo_t *bootInfo) {
	initializeScreen(bootInfo->frameBuffer, bootInfo->initialFont, 0x00000088, 0xFFFFFFFF);
	k_print("Some string\n");
	k_print("some other string\n");
	return;
}