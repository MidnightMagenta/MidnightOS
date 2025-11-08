#ifndef _NYX_LINKAGE_H
#define _NYX_LINKAGE_H

#include "compiler.h"

//FIXME: generic page size
#define __page_aligned_bss __section(".bss") __align(4096)

#endif