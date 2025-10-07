#ifndef MDOSBOOT_UTILS_H
#define MDOSBOOT_UTILS_H

#define ALIGN_UP(val, alignment, castType) ((castType) val + ((castType) alignment - 1)) & (~((castType) alignment - 1))
#define ALIGN_DOWN(val, alignment, castType) ((castType) val & ~((castType) alignment - 1))

#endif