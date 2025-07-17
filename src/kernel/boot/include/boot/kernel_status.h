#ifndef MDOS_KERNEL_STATUS_H
#define MDOS_KERNEL_STATUS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <thread/atomic.h>

typedef enum {
	MDOS_STATUS_FLAG_PMM_AVAIL,
	MDOS_STATUS_FLAG_VMM_AVAIL,
	MDOS_STATUS_FLAG_VMALLOC_AVAIL,
	MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL
} KernelStatusFlag;

typedef struct {
	mdos_atomic_flag PMM_Avail;
	mdos_atomic_flag VMM_Avail;
	mdos_atomic_flag vmallocAvail;
	mdos_atomic_flag bucketAllocatorAvail;
} KernelStatus;

void mdos_set_status_flag(KernelStatusFlag flag, bool value);
bool mdos_get_status_flag(KernelStatusFlag flag);

#ifdef __cplusplus
}
#endif
#endif