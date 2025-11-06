#ifndef _NYX_RESULT_H
#define _NYX_RESULT_H

#include <nyx/types.h>

typedef enum nyx_result {
    NYX_RES_SUCCESS,
    NYX_RES_MIN_ERROR = 0x80000000,
    NYX_RES_INIT_FAIL,
    NYX_RES_OUT_OF_RESOURCES,
} nyx_result_t;

#define NYX_ERROR(v) (((s32) v) < 0)

#endif