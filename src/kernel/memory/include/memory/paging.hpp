#ifndef MDOS_PAGING_H
#define MDOS_PAGING_H

#include <boot/boot_info.hpp>
#include <k_utils/result.h>
#include <k_utils/types.h>
#include <libk/string.h>
#include <memory/phys_virt_conversion.h>
#include <stdint.h>
#include <thread/lock_guard.hpp>
#include <thread/spinlock.hpp>

namespace MdOS::mem::virt {
enum class EntryControlBit {
	PagePresent,
	ReadWrite,
	UserAccessible,
	WriteThrough,
	CacheDisable,
	Accessed,
	Dirty,
	PageSize,
	Global,
	IMPL_RES1,
	IMPL_RES2,
	IMPL_RES3,
	PageAttributeTable,
	NoExecute
};

enum EntryType : uint8_t { PML5E = 0b101, PML4E = 0b100, PDPE = 0b011, PDE = 0b010, PTE = 0b001, INVALID = 0b000 };

enum EntryFlagBits : uint16_t {
	ReadWrite = 1 << 0,
	UserAccessible = 1 << 1,
	NoExecute = 1 << 2,
	WriteThrough = 1 << 3,
	CacheDisable = 1 << 4,
	Global = 1 << 5,
	Page2MiB = 1 << 6,
	Page1GiB = 1 << 7,
	PAT = 1 << 8,
};

using PageEntry = uint64_t;

static constexpr size_t pageSize4KiB = 0x1000;
static constexpr size_t pageSize2MiB = 0x200000;
static constexpr size_t pageSize1GiB = 0x40000000;
static constexpr size_t pageSizePML4 = 0x8000000000;
static constexpr size_t pageTableSize = 0x1000;

void set_type(EntryType type, PageEntry *entry);
EntryType get_type(PageEntry *entry);
void set_bit(EntryControlBit bit, bool value, PageEntry *entry);
bool get_bit(EntryControlBit bit, PageEntry *entry);
void set_addr(uint64_t addr, PageEntry *entry);
uint64_t get_addr(PageEntry *entry);

struct Entry {
	PageEntry m_entry;

	inline void set_type(EntryType type) { MdOS::mem::virt::set_type(type, &m_entry); }
	inline EntryType get_type() { return MdOS::mem::virt::get_type(&m_entry); }
	inline void set_bit(EntryControlBit bit, bool value) { MdOS::mem::virt::set_bit(bit, value, &m_entry); }
	inline bool get_bit(EntryControlBit bit) { return MdOS::mem::virt::get_bit(bit, &m_entry); }
	inline void set_addr(uint64_t addr) { MdOS::mem::virt::set_addr(addr, &m_entry); }
	inline uint64_t get_addr() { return MdOS::mem::virt::get_addr(&m_entry); }
} __attribute__((aligned(alignof(uint64_t))));

static_assert(sizeof(Entry) == sizeof(PageEntry));

inline void set_cr3(uint64_t pml4) { __asm__ volatile("mov %0, %%cr3;" ::"r"(pml4) : "memory"); }
inline void invalidate_page(uintptr_t vaddr) { __asm__ volatile("invlpg (%0)" ::"r"(vaddr) : "memory"); }

class VirtualMemoryManagerPML4 {
public:
	VirtualMemoryManagerPML4() {}
	~VirtualMemoryManagerPML4() {}

	Result init();
	Result init(Entry *pml4);
	Result init(VirtualMemoryManagerPML4 *vmm);
	Result map_range(PhysicalAddress paddrBase, VirtualAddress vaddrBase, size_t size, EntryFlagBits flags);
	Result unmap_range(VirtualAddress vaddrBase, size_t size);
	Result swap_attributes(VirtualAddress vaddr, EntryFlagBits newFlags);

	PhysicalAddress query_paddr(VirtualAddress vaddr);

	inline Entry *get_pml4() { return m_pml4; }
	inline void activate() {
		m_boundVMM = this;
		set_cr3(MDOS_VIRT_TO_PHYS(uint64_t(m_pml4)));
	}

	static VirtualMemoryManagerPML4 *get_active() { return m_boundVMM; }

#ifdef _DEBUG
	static void print_entry(Entry entry);
	void dump_pml4(bool presentOnly);
	void dump_translation_hierarchy(VirtualAddress vaddr);
	// TODO: implement full page table dump
#endif

private:
	bool table_empty(Entry *table);
	void free_table_if_empty(Entry *table, Entry &entry);
	Entry *get_entry(Entry *table, size_t index, EntryType type);
	Result map_4KiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);
	Result map_2MiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);
	Result map_1GiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);

	MdOS::thread::Spinlock *get_lock() { return &m_lock; }

	static VirtualMemoryManagerPML4 *m_boundVMM;
	Entry *m_pml4 = nullptr;
	MdOS::thread::Spinlock m_lock;
};

Result map_kernel(SectionInfo *sections, size_t sectionInfoCount, MemMap *memMap, BootstrapMemoryRegion bootHeap,
				  GOPFramebuffer *framebuffer, VirtualMemoryManagerPML4 *vmm);

inline VirtualMemoryManagerPML4 *g_krnlVMM = nullptr;
}// namespace MdOS::mem::virt

#endif