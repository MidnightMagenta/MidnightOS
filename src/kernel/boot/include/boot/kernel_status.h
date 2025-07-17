#ifndef MDOS_KERNEL_STATUS_H
#define MDOS_KERNEL_STATUS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
	MDOS_STATUS_FLAG_PMM_AVAIL,
	MDOS_STATUS_FLAG_VMM_AVAIL,
	MDOS_STATUS_FLAG_VMALLOC_AVAIL,
	MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL
} KernelStatusFlag;

typedef struct {
	bool PMM_Avail;
	bool VMM_Avail;
	bool vmallocAvail;
	bool bucketAllocatorAvail;
} KernelStatus;

void mdos_set_status_flag(KernelStatusFlag flag, bool value);
bool mdos_get_status_flag(KernelStatusFlag flag);

#ifdef __cplusplus
}
#endif
#endif