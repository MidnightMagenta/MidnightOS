#ifndef _INIT_H
#define _INIT_H

#include <nyx/compiler.h>

#define __init      __section(".init.text")
#define __initdata  __section(".init.data")
#define __initconst __section(".init.rodata")
#define __initzero  __section(".init.bss")

#endif