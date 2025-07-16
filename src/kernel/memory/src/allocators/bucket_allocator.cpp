// #include <k_utils/stack.hpp>
// #include <libk/stdlib.h>
// #include <memory/allocators/bucket_allocator.hpp>
// #include <memory/new.hpp>
// #include <memory/paging.hpp>
// #include <memory/pmm.hpp>

// #define INITIAL_HEAP_SIZE 524288
// #define MIN_HEAP_ORDER 3
// #define MAX_HEAP_ORDER 11

// uintptr_t heapVaddrTop = 0xFFFFFEFF80000000ULL;
// utils::Stack<uintptr_t> *freeLists[MAX_HEAP_ORDER - MIN_HEAP_ORDER];

// int fast_u64_log2(uint64_t value) { return 64 - __builtin_clzll(value - 1); }

// static void rebuild_stacks() {}

// static inline utils::Stack<uintptr_t> *get_free_list(uint8_t order) { return freeLists[order - MIN_HEAP_ORDER]; }

// static Result build_list(size_t bucketSize, uint8_t order) {
// 	MdOS::mem::phys::PhysicalMemoryAllocation allocation;
// 	MdOS::mem::phys::alloc_pages(INITIAL_HEAP_SIZE / 0x1000, &allocation);
// 	if (allocation.base == 0 || allocation.numPages == 0) { return MDOS_INIT_FAILURE; };

// 	for (size_t i = 0; i < allocation.numPages; i++) {
// 		MdOS::mem::phys::set_page_bucket_size((allocation.base + i) * 0x1000, order);
// 	}

// 	MdOS::mem::virt::g_defaultVMM->map_range(
// 			allocation.base, heapVaddrTop, INITIAL_HEAP_SIZE, MdOS::mem::virt::ReadWrite);

// 	for (size_t i = 0; i < INITIAL_HEAP_SIZE; i += bucketSize) {
// 		get_free_list(order)->push(heapVaddrTop + (i * bucketSize));
// 	}
// 	heapVaddrTop += INITIAL_HEAP_SIZE;
// }

// Result MdOS::mem::bucket_heap_init() {
// 	const size_t freeListCount = MAX_HEAP_ORDER - MIN_HEAP_ORDER;
// 	for (size_t i = 0; i < freeListCount; i++) {
// 		size_t bucketSize = 1ULL << (MIN_HEAP_ORDER + i);
// 		void *mem = malloc(sizeof(utils::Stack<uintptr_t>));
// 		freeLists[i] = new (mem) utils::Stack<uintptr_t>(INITIAL_HEAP_SIZE / bucketSize);
// 		build_list(bucketSize, i + MIN_HEAP_ORDER);
// 	}
// }

// void *MdOS::mem::_internal_alloc(size_t size) { return nullptr; }
// void MdOS::mem::_internal_free(void *addr) {}