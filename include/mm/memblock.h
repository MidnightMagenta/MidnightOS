#ifndef _MEMBLOCK_H
#define _MEMBLOCK_H

#include <nyx/init.h>
#include <nyx/types.h>
#include <stddef.h>

struct memblock_region {
    phys_addr_t base;
    phys_addr_t size;
};

struct memblock_type {
    size_t                  cnt;
    size_t                  max;
    phys_addr_t             total_size;
    struct memblock_region *regions;
};

struct memblock {
    bool                 bottom_up;
    struct memblock_type memory;
    struct memblock_type reserved;
};

extern struct memblock memblock;

#endif