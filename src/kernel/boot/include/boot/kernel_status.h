#ifndef MDOS_KERNEL_STATUS_H
#define MDOS_KERNEL_STATUS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

int MdOS_krnlStatus_kernelReady = false;
int MdOS_krnlStatus_globalObjectsReady = false;

#ifdef __cplusplus
}
#endif
#endif