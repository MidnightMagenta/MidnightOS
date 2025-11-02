#ifndef _MDOS_RESULT_H
#define _MDOS_RESULT_H

#include <mdos/types.h>

typedef enum mdos_result {
    MDOS_RES_SUCCESS,
    MDOS_RES_MIN_ERROR = 0x80000000,
    MDOS_RES_INIT_FAIL,
    MDOS_RES_OUT_OF_RESOURCES,
} mdos_result_t;

#define MDOS_ERROR(v) (((__s32) v) < 0)

#endif