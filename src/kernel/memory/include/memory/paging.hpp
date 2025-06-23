#ifndef PAGING_H
#define PAGING_H

#include <IO/debug_print.hpp>
#include <memory/memory_types.hpp>
#include <stdint.h>

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
static_assert(sizeof(Entry) == 8, "Entry must be 8 bytes");

struct PageTable {
	Entry m_entries[512];
} __attribute__((aligned(0x1000)));
static_assert(sizeof(PageTable) == 0x1000);
static_assert(alignof(PageTable) == 0x1000);

}// namespace MdOS::Memory::Paging

#endif