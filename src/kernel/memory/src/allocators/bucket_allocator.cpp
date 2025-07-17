#include <IO/debug_print.h>
#include <boot/kernel_status.h>
#include <k_utils/stack.hpp>
#include <libk/stdlib.h>
#include <memory/allocators/bucket_allocator.hpp>
#include <memory/new.hpp>
#include <memory/paging.hpp>
#include <memory/pmm.hpp>

#define INITIAL_HEAP_SIZE 49152

utils::Stack<uintptr_t> *freeLists[(MAX_HEAP_ORDER - MIN_HEAP_ORDER) + 1];

static inline utils::Stack<uintptr_t> *get_free_list(uint8_t order) { return freeLists[order - MIN_HEAP_ORDER]; }

static Result build_list(size_t bucketSize, uint8_t order, uintptr_t *heapVaddrTop) {
	LOG_FUNC_ENTRY;

	MdOS::mem::phys::PhysicalMemoryAllocation allocation;
	MdOS::mem::phys::alloc_pages(INITIAL_HEAP_SIZE / 0x1000, &allocation);
	if (allocation.base == 0 || allocation.numPages == 0) { return MDOS_INIT_FAILURE; };

	for (size_t i = 0; i < allocation.numPages; i++) {
		MdOS::mem::phys::set_page_bucket_size((allocation.base + (i * 0x1000)), order);
	}

	MdOS::mem::virt::VirtualMemoryManagerPML4::get_active()->map_range(allocation.base, *heapVaddrTop,
																	   INITIAL_HEAP_SIZE, MdOS::mem::virt::ReadWrite);

	for (size_t i = 0; i < INITIAL_HEAP_SIZE; i += bucketSize) {
		get_free_list(order)->push(*heapVaddrTop + (i * bucketSize));
	}
	*heapVaddrTop += INITIAL_HEAP_SIZE;

	DEBUG_LOG_VB3("Initialized free list of order %u. List size %lu\n", order, get_free_list(order)->size());
	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::bucket_heap_init() {
	LOG_FUNC_ENTRY;

	const size_t freeListCount = (MAX_HEAP_ORDER - MIN_HEAP_ORDER) + 1;
	uintptr_t heapVaddrTop = 0xFFFFFEFF80000000ULL;
	for (size_t i = 0; i < freeListCount; i++) {
		size_t bucketSize = 1ULL << (MIN_HEAP_ORDER + i);
		void *mem = malloc(sizeof(utils::Stack<uintptr_t>));
		freeLists[i] = new (mem) utils::Stack<uintptr_t>(INITIAL_HEAP_SIZE / bucketSize);
		if (build_list(bucketSize, (uint8_t) (i + MIN_HEAP_ORDER), &heapVaddrTop) != MDOS_SUCCESS) {
			return MDOS_INIT_FAILURE;
		};
	}

	mdos_set_status_flag(MDOS_STATUS_FLAG_PMM_AVAIL, true);

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

void *MdOS::mem::_internal_alloc(size_t size) {
	uint8_t order = (uint8_t) fast_u64_ceil_log2(size);
	order = order < MIN_HEAP_ORDER ? order : MIN_HEAP_ORDER;
	ALLOC_LOG("Allocating %u bytes. Bucket order %u", size, order);
	return order <= MAX_HEAP_ORDER ? (void *) get_free_list(order)->pop() : nullptr;
}

bool MdOS::mem::_internal_free(void *addr) {
	uint8_t order = MdOS::mem::phys::get_page_bucket_size(
			MdOS::mem::virt::VirtualMemoryManagerPML4::get_active()->query_paddr((VirtualAddress) addr));
	if (order >= MIN_HEAP_ORDER && order <= MAX_HEAP_ORDER) {
		ALLOC_LOG("Freeing address 0x%lx of order %u", addr, order);
		get_free_list(order)->push((uintptr_t) addr);
		return true;
	} else {
		ALLOC_LOG("Page containing address 0x%lx not bucket-heap managed", addr);
		return false;
	}
}