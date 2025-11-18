#include <mm/memblock.h>

#define INIT_MEMBLOCK_REGIONS 128

static struct memblock_region memblock_memory_init_regions[INIT_MEMBLOCK_REGIONS] __initdata;
static struct memblock_region memblock_reserved_init_regions[INIT_MEMBLOCK_REGIONS] __initdata;

struct memblock memblock __initdata = {
        .memory.regions = memblock_memory_init_regions,
        .memory.cnt     = 1,
        .memory.max     = INIT_MEMBLOCK_REGIONS,

        .reserved.regions = memblock_reserved_init_regions,
        .reserved.cnt     = 1,
        .reserved.max     = INIT_MEMBLOCK_REGIONS,

        .bottom_up = false,
};