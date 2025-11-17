#ifndef _NYX_LINKAGE_H
#define _NYX_LINKAGE_H

#include <asm/page.h>
#include <nyx/compiler.h>

#define __page_aligned_bss __section(".bss") __align(PAGE_SIZE)

#endif