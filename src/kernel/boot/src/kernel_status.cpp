#include <boot/kernel_status.h>
#include <thread/spinlock.h>

inline KernelStatus _internal_krnlStatus = {false, false, false, false};

extern "C" void mdos_set_status_flag(KernelStatusFlag flag, bool value) {
	switch (flag) {
		case MDOS_STATUS_FLAG_PMM_AVAIL:
			if (value) {
				atomic_flag_test_and_set(&_internal_krnlStatus.PMM_Avail);
			} else {
				atomic_flag_clear(&_internal_krnlStatus.PMM_Avail);
			}
			return;
		case MDOS_STATUS_FLAG_VMM_AVAIL:
			if (value) {
				atomic_flag_test_and_set(&_internal_krnlStatus.VMM_Avail);
			} else {
				atomic_flag_clear(&_internal_krnlStatus.VMM_Avail);
			}
			return;
		case MDOS_STATUS_FLAG_VMALLOC_AVAIL:
			if (value) {
				atomic_flag_test_and_set(&_internal_krnlStatus.vmallocAvail);
			} else {
				atomic_flag_clear(&_internal_krnlStatus.vmallocAvail);
			}
			return;
		case MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL:
			if (value) {
				atomic_flag_test_and_set(&_internal_krnlStatus.bucketAllocatorAvail);
			} else {
				atomic_flag_clear(&_internal_krnlStatus.bucketAllocatorAvail);
			}
			return;
		default:
			return;
	}
}

extern "C" bool mdos_get_status_flag(KernelStatusFlag flag) {
	switch (flag) {
		case MDOS_STATUS_FLAG_PMM_AVAIL:
			return atomic_flag_test(&_internal_krnlStatus.PMM_Avail);
		case MDOS_STATUS_FLAG_VMM_AVAIL:
			return atomic_flag_test(&_internal_krnlStatus.VMM_Avail);
		case MDOS_STATUS_FLAG_VMALLOC_AVAIL:
			return atomic_flag_test(&_internal_krnlStatus.vmallocAvail);
		case MDOS_STATUS_FLAG_BUCKET_HEAP_AVAIL:
			return atomic_flag_test(&_internal_krnlStatus.bucketAllocatorAvail);
		default:
			return false;
	}
}