#include <memory/paging.hpp>
#include <IO/debug_print.h>

void MdOS::memory::paging::set_type(EntryType type, PageEntry *entry) {
	*entry &= ~uint64_t(0xE00);
	*entry |= uint64_t(type) << 9;
}

MdOS::memory::paging::EntryType MdOS::memory::paging::get_type(PageEntry *entry) {
	return EntryType(uint8_t(*entry >> 9) & uint8_t(0b0111));
}

void MdOS::memory::paging::set_bit(EntryControlBit bit, bool value, PageEntry *entry) {
	EntryType type = get_type(entry);
	switch (bit) {
		case EntryControlBit::PagePresent:
			if (value == true) {
				*entry |= 1ULL << 0;
			} else {
				*entry &= ~(1ULL << 0);
			}
			return;
		case EntryControlBit::ReadWrite:
			if (value == true) {
				*entry |= 1ULL << 1;
			} else {
				*entry &= ~(1ULL << 1);
			}
			return;
		case EntryControlBit::UserAccessible:
			if (value == true) {
				*entry |= 1ULL << 2;
			} else {
				*entry &= ~(1ULL << 2);
			}
			return;
		case EntryControlBit::WriteThrough:
			if (value == true) {
				*entry |= 1ULL << 3;
			} else {
				*entry &= ~(1ULL << 3);
			}
			return;
		case EntryControlBit::CacheDisable:
			if (value == true) {
				*entry |= 1ULL << 4;
			} else {
				*entry &= ~(1ULL << 4);
			}
			return;
		case EntryControlBit::Accessed:
			if (value == true) {
				*entry |= 1ULL << 5;
			} else {
				*entry &= ~(1ULL << 5);
			}
			return;
		case EntryControlBit::Dirty:
			if (value == true) {
				*entry |= 1ULL << 6;
			} else {
				*entry &= ~(1ULL << 6);
			}
			return;
		case EntryControlBit::PageSize:
			if (value == true) {
				*entry |= 1ULL << 7;
			} else {
				*entry &= ~(1ULL << 7);
			}
			return;
		case EntryControlBit::Global:
			if (value == true) {
				*entry |= 1ULL << 8;
			} else {
				*entry &= ~(1ULL << 8);
			}
			return;
		case EntryControlBit::IMPL_RES1:
			PRINT_ERROR("Attempted to set reserved bit 9");
			return;
		case EntryControlBit::IMPL_RES2:
			PRINT_ERROR("Attempted to set reserved bit 10");
			return;
		case EntryControlBit::IMPL_RES3:
			PRINT_ERROR("Attempted to set reserved bit 11");
			return;
		case EntryControlBit::PageAttributeTable:
			if (type == EntryType::INVALID || type > EntryType::PML5E) {
				PRINT_ERROR("Attempted to read an invalid entry");
			}
			if (type == EntryType::PTE) {
				if (value == true) {
					*entry |= 1ULL << 7;
				} else {
					*entry &= ~(1ULL << 7);
				}
			} else if (type == EntryType::PDE || type == EntryType::PDPE) {
				if (get_bit(EntryControlBit::PageSize, entry)) {
					if (value == true) {
						*entry |= 1ULL << 12;
					} else {
						*entry &= ~(1ULL << 12);
					}
				} else {
					PRINT_ERROR("Attempted to set PTE bit in a non-large PDE or PDPE");
				}
			} else {
				PRINT_ERROR("Attempted to set an invalid bit");
			}
			return;
		case EntryControlBit::NoExecute:
			if (value == true) {
				*entry |= 1ULL << 63;
			} else {
				*entry &= ~(1ULL << 63);
			}
			return;
		default:
			PRINT_ERROR("Invalid page entry type");
			return;
	}
}

bool MdOS::memory::paging::get_bit(EntryControlBit bit, PageEntry *entry) {
	EntryType type = get_type(entry);
	switch (bit) {
		case EntryControlBit::PagePresent:
			return *entry & 1ULL << 0;
		case EntryControlBit::ReadWrite:
			return *entry & 1ULL << 1;
		case EntryControlBit::UserAccessible:
			return *entry & 1ULL << 2;
		case EntryControlBit::WriteThrough:
			return *entry & 1ULL << 3;
		case EntryControlBit::CacheDisable:
			return *entry & 1ULL << 4;
		case EntryControlBit::Accessed:
			return *entry & 1ULL << 5;
		case EntryControlBit::Dirty:
			return *entry & 1ULL << 6;
		case EntryControlBit::PageSize:
			return *entry & 1ULL << 7;
		case EntryControlBit::Global:
			return *entry & 1ULL << 8;
		case EntryControlBit::IMPL_RES1:
			return *entry & 1ULL << 9;
		case EntryControlBit::IMPL_RES2:
			return *entry & 1ULL << 10;
		case EntryControlBit::IMPL_RES3:
			return *entry & 1ULL << 11;
		case EntryControlBit::PageAttributeTable:
			if (type == EntryType::INVALID || type > EntryType::PML5E) {
				PRINT_ERROR("Attempted to read an invalid entry");
			}
			if (type == EntryType::PTE) {
				return *entry & 1ULL << 7;
			} else if (type == EntryType::PDE || type == EntryType::PDPE) {
				if (get_bit(EntryControlBit::PageSize, entry)) {
					return *entry & 1ULL << 12;
				} else {
					PRINT_INFO("Attempted to get PTE bit in a non-large PDE or PDPE");
					return false;
				}
			} else {
				PRINT_INFO("Attempted to get an invalid bit");
				return false;
			}
		case EntryControlBit::NoExecute:
			return *entry & 1ULL << 63;
		default:
			PRINT_INFO("Invalid page entry type");
			return false;
	}
}

void MdOS::memory::paging::set_addr(uint64_t addr, PageEntry *entry) {
	*entry &= ~0x000FFFFFFFFFF000ULL;
	*entry |= (addr & ~0xFFFULL);
}

uint64_t MdOS::memory::paging::get_addr(PageEntry *entry) { return *entry & 0x000FFFFFFFFFF000ULL; }