#ifndef MDOS_PAGING_INDEX_HELPER_H
#define MDOS_PAGING_INDEX_HELPER_H

#include <k_utils/types.h>
#include <stddef.h>

namespace MdOS::Memory::Paging {
inline constexpr uint16_t pml5_index(VirtualAddress addr) { return (addr >> 48) & 0x1FF; }
inline constexpr uint16_t pml4_index(VirtualAddress addr) { return (addr >> 39) & 0x1FF; }
inline constexpr uint16_t pdp_index(VirtualAddress addr) { return (addr >> 30) & 0x1FF; }
inline constexpr uint16_t pd_index(VirtualAddress addr) { return (addr >> 21) & 0x1FF; }
inline constexpr uint16_t pt_index(VirtualAddress addr) { return (addr >> 12) & 0x1FF; }
inline constexpr uint16_t offset(VirtualAddress addr) { return addr & 0xFFF; }

template<size_t CannonicalBit>
inline constexpr uintptr_t sign_exted(uintptr_t addr) {
	if (addr & (1ULL << CannonicalBit)) { addr |= ~((1ULL << (CannonicalBit + 1)) - 1); }
	return addr;
}

inline constexpr uintptr_t make_4_level_addr(uint16_t pml4, uint16_t pdp, uint16_t pd, uint16_t pt, uint16_t offset) {
	uintptr_t addr = 0;
	addr |= uintptr_t(pml4 & 0x1FF) << 39;
	addr |= uintptr_t(pdp & 0x1FF) << 30;
	addr |= uintptr_t(pd & 0x1FF) << 21;
	addr |= uintptr_t(pt & 0x1FF) << 12;
	addr |= uintptr_t(offset & 0xFFF);
	addr = sign_exted<47>(addr);
	return addr;
}
inline constexpr uintptr_t make_5_level_addr(uint16_t pml5, uint16_t pml4, uint16_t pdp, uint16_t pd, uint16_t pt,
											 uint16_t offset) {
	uintptr_t addr = 0;
	addr |= uintptr_t(pml5 & 0x1FF) << 48;
	addr |= uintptr_t(pml4 & 0x1FF) << 39;
	addr |= uintptr_t(pdp & 0x1FF) << 30;
	addr |= uintptr_t(pd & 0x1FF) << 21;
	addr |= uintptr_t(pt & 0x1FF) << 12;
	addr |= uintptr_t(offset & 0xFFF);
	addr = sign_exted<56>(addr);
	return addr;
}
}// namespace MdOS::Memory::Paging

#endif