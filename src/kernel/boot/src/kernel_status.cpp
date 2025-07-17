#include <boot/kernel_status.h>
#include <thread/spinlock.h>

inline mdos_spinlock _internal_krnlStatus_lock;
inline KernelStatus _internal_krnlStatus = {false, false, false, false};

extern "C" void mdos_set_status_flag(KernelStatusFlag flag, bool value) {
	switch (flag) {
		case MDOS_STATUS_FLAG_PMM_AVAIL:
			_internal_krnlStatus.PMM_Avail = value;
			return;
		case MDOS_STATUS_FLAG_VMM_AVAIL:
			_internal_krnlStatus.VMM_Avail = value;
			return;
		case MDOS_STATUS_FLAG_VMALLOC_AVAIL:
			_internal_krnlStatus.vmallocAvail = value;
			return;
		case MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL:
			_internal_krnlStatus.bucketAllocatorAvail = value;
			return;
		default:
			return;
	}
}

extern "C" bool mdos_get_status_flag(KernelStatusFlag flag) {
	switch (flag) {
		case MDOS_STATUS_FLAG_PMM_AVAIL:
			return _internal_krnlStatus.PMM_Avail;
		case MDOS_STATUS_FLAG_VMM_AVAIL:
			return _internal_krnlStatus.VMM_Avail;
		case MDOS_STATUS_FLAG_VMALLOC_AVAIL:
			return _internal_krnlStatus.vmallocAvail;
		case MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL:
			return _internal_krnlStatus.bucketAllocatorAvail;
		default:
			return false;
	}
}