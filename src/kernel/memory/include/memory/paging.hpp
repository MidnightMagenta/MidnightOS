#ifndef MDOS_PAGING_H
#define MDOS_PAGING_H

#include <IO/debug_print.h>
#include <boot/boot_info.hpp>
#include <k_utils/result.hpp>
#include <k_utils/types.h>
#include <stdint.h>

#define MDOS_MEMORY_DIRECT_MAP_REGION_BASE 0xFFFF800000000000ULL
#define MDOS_MEMORY_DIRECT_MAP_REGION_END 0xFFFF880000000000ULL
#define MDOS_VIRT_TO_PHYS(vaddr) (vaddr - MDOS_MEMORY_DIRECT_MAP_REGION_BASE)
#define MDOS_PHYS_TO_VIRT(paddr) (paddr + MDOS_MEMORY_DIRECT_MAP_REGION_BASE)

namespace MdOS::Memory::Paging {
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

enum class EntryType : uint8_t {
	PML5E = 0b101,
	PML4E = 0b100,
	PDPE = 0b011,
	PDE = 0b010,
	PTE = 0b001,
	INVALID = 0b000
};

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

void set_type(EntryType type, PageEntry *entry);
EntryType get_type(PageEntry *entry);
void set_bit(EntryControlBit bit, bool value, PageEntry *entry);
bool get_bit(EntryControlBit bit, PageEntry *entry);
void set_addr(uint64_t addr, PageEntry *entry);
uint64_t get_addr(PageEntry *entry);

struct Entry {
	PageEntry m_entry;

	inline void set_type(EntryType type) { MdOS::Memory::Paging::set_type(type, &m_entry); }
	inline EntryType get_type() { return MdOS::Memory::Paging::get_type(&m_entry); }
	inline void set_bit(EntryControlBit bit, bool value) { MdOS::Memory::Paging::set_bit(bit, value, &m_entry); }
	inline bool get_bit(EntryControlBit bit) { return MdOS::Memory::Paging::get_bit(bit, &m_entry); }
	inline void set_addr(uint64_t addr) { MdOS::Memory::Paging::set_addr(addr, &m_entry); }
	inline uint64_t get_addr() { return MdOS::Memory::Paging::get_addr(&m_entry); }
} __attribute__((aligned(alignof(uint64_t))));

static_assert(sizeof(Entry) == sizeof(PageEntry));

inline void set_cr3(uint64_t pml4) { __asm__ volatile("mov %0, %%cr3;" ::"r"(pml4) : "memory"); }
inline void invalidate_page(uintptr_t vaddr) { __asm__ volatile("invlpg (%0)" ::"r"(vaddr) : "memory"); }

static constexpr size_t pageSize4KiB = 0x1000;
static constexpr size_t pageSize2MiB = 0x200000;
static constexpr size_t pageSize1GiB = 0x40000000;
static constexpr size_t pageTableSize = 0x1000;

class VirtualMemoryManagerPML4 {
public:
	VirtualMemoryManagerPML4() {}
	~VirtualMemoryManagerPML4() {}

	MdOS::Result init();
	MdOS::Result init(Entry *pml4);
	MdOS::Result init(VirtualMemoryManagerPML4 *vmm);
	MdOS::Result map_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);
	MdOS::Result unmap_page(VirtualAddress vaddr);
	MdOS::Result map_range(PhysicalAddress paddrBase, VirtualAddress vaddrBase, size_t numPages, EntryFlagBits flags);
	MdOS::Result unmap_range(VirtualAddress vaddrBase, size_t numPages);
	MdOS::Result swap_attributes(VirtualAddress vaddr, EntryFlagBits newFlags);

	inline Entry *get_pml4() { return m_pml4; }
	inline void activate() { set_cr3(MDOS_VIRT_TO_PHYS(uint64_t(m_pml4))); }

	static inline void bind_vmm(VirtualMemoryManagerPML4 *vmm) { m_boundVMM = vmm; }
	static inline VirtualMemoryManagerPML4 *get_bound_vmm() { return m_boundVMM; }

private:
	bool table_empty(Entry *table);
	Entry *get_entry(Entry *table, size_t index, EntryType type);
	MdOS::Result map_4KiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);
	MdOS::Result map_2MiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);
	MdOS::Result map_1GiB_page(PhysicalAddress paddr, VirtualAddress vaddr, EntryFlagBits flags);

	static VirtualMemoryManagerPML4 *m_boundVMM;
	Entry *m_pml4 = nullptr;
};

MdOS::Result map_kernel(SectionInfo *sections, size_t sectionInfoCount, MemMap *memMap, BootstrapMemoryRegion bootHeap, GOPFramebuffer* framebuffer,
						VirtualMemoryManagerPML4 *vmm);

static VirtualMemoryManagerPML4 g_defaultVMM;
}// namespace MdOS::Memory::Paging

#endif