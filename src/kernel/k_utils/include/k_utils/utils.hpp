#ifndef MDOS_K_UTILS_H
#define MDOS_K_UTILS_H

#define ROUND_NTH(val, n) ((val + n - 1) / n) * n
#define ALIGN_ADDR(val, alignment, castType) (castType(val) + (castType(alignment) - 1)) & (~(castType(alignment) - 1))
#define ALIGN_UP(val, alignment, castType) (castType(val) + (castType(alignment) - 1)) & (~(castType(alignment) - 1))

#endif