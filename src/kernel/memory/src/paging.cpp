#include <error/panic.h>
#include <k_utils/memory.h>
#include <memory/paging.hpp>
#include <memory/paging_index_helper.hpp>
#include <memory/pmm.hpp>

#include <IO/debug_print.h>
#include <k_utils/utils.hpp>

void MdOS::Memory::Paging::set_type(EntryType type, PageEntry *entry) {
	*entry &= ~uint64_t(0xE00);
	*entry |= uint64_t(type) << 9;
}

MdOS::Memory::Paging::EntryType MdOS::Memory::Paging::get_type(PageEntry *entry) {
	return EntryType(uint8_t(*entry >> 9) & uint8_t(0b0111));
}

void MdOS::Memory::Paging::set_bit(EntryControlBit bit, bool value, PageEntry *entry) {
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

bool MdOS::Memory::Paging::get_bit(EntryControlBit bit, PageEntry *entry) {
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

void MdOS::Memory::Paging::set_addr(uint64_t addr, PageEntry *entry) {
	*entry &= ~0x000FFFFFFFFFF000ULL;
	*entry |= (addr & ~0xFFFULL);
}

uint64_t MdOS::Memory::Paging::get_addr(PageEntry *entry) { return *entry & 0x000FFFFFFFFFF000ULL; }

MdOS::Memory::Paging::VirtualMemoryManagerPML4 *MdOS::Memory::Paging::VirtualMemoryManagerPML4::m_boundVMM = nullptr;

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::init() {
	if (m_pml4 != nullptr) { return MdOS::Result::ALREADY_INITIALIZED; }
	MdOS::Memory::PMM::PhysicalMemoryAllocation allocation;
	MdOS::Result res = MdOS::Memory::PMM::alloc_pages(&allocation);
	if (res != MdOS::Result::SUCCESS) {
		DEBUG_LOG("Failed to allocate PML4 with: %u", uint32_t(res));
		return res;
	}
	m_pml4 = (Entry *) (MDOS_PHYS_TO_VIRT(allocation.base));
	memset((void *) m_pml4, 0x0, MdOS::Memory::Paging::pageSize4KiB);

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::init(Entry *pml4) {
	if (pml4 == nullptr) { return MdOS::Result::INIT_FAILURE; }
	m_pml4 = pml4;
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::init(VirtualMemoryManagerPML4 *vmm) {
	MdOS::Result res = this->init();
	if (res != MdOS::Result::SUCCESS || m_pml4 == nullptr) { return res; }
	memcpy(m_pml4, vmm->get_pml4(), pageTableSize);
	//TODO: implement copy initialization
	return MdOS::Result::SUCCESS;
}

bool MdOS::Memory::Paging::VirtualMemoryManagerPML4::table_empty(Entry *table) {
	for (size_t i = 0; i < 512; i++) {
		if (table[i].get_bit(EntryControlBit::PagePresent)) { return false; }
	}
	return true;
}

MdOS::Memory::Paging::Entry *MdOS::Memory::Paging::VirtualMemoryManagerPML4::get_entry(Entry *table, size_t index,
																					   EntryType type) {
	Entry ent = table[index];
	if (!ent.get_bit(EntryControlBit::PagePresent)) {
		ent = {0ULL};
		ent.set_type(type);
		ent.set_bit(EntryControlBit::PagePresent, true);
		ent.set_bit(EntryControlBit::ReadWrite, true);
		ent.set_bit(EntryControlBit::UserAccessible, true);
		ent.set_addr(MdOS::Memory::PMM::alloc_page());
		memset((void *) (MDOS_PHYS_TO_VIRT(ent.get_addr())), 0x0, MdOS::Memory::Paging::pageSize4KiB);
		table[index] = ent;
	}
	kassert(ent.get_addr() != 0);
	return (Entry *) MDOS_PHYS_TO_VIRT(ent.get_addr());
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_page(PhysicalAddress paddr, VirtualAddress vaddr,
																	  EntryFlagBits flags) {
	MdOS::Result res;
	if (m_pml4 == nullptr) {
		res = init();
		if (res != MdOS::Result::SUCCESS) { return res; }
	}
	kassert(m_pml4 != nullptr);
	if (!is_canonical_pml4(vaddr)) { return MdOS::Result::INVALID_PARAMETER; }

	if (flags & EntryFlagBits::Page2MiB) {
		res = map_2MiB_page(paddr, vaddr, flags);
	} else if (flags & EntryFlagBits::Page1GiB) {
		res = map_1GiB_page(paddr, vaddr, flags);
	} else {
		res = map_4KiB_page(paddr, vaddr, flags);
	}

	if (res == MdOS::Result::SUCCESS) { invalidate_page(vaddr); }

	return res;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::unmap_page(VirtualAddress vaddr) {
	MdOS::Result res;
	if (m_pml4 == nullptr) {
		res = init();
		if (res != MdOS::Result::SUCCESS) { return res; }
	}
	kassert((vaddr % MdOS::Memory::Paging::pageSize4KiB) == 0);
	if (!is_canonical_pml4(vaddr)) { return MdOS::Result::INVALID_PARAMETER; }

	Entry &pml4e = m_pml4[pml4_index(vaddr)];
	if (!pml4e.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::SUCCESS; }

	Entry *pdp = (Entry *) MDOS_PHYS_TO_VIRT(pml4e.get_addr());
	Entry &pdpe = pdp[pdp_index(vaddr)];
	if (!pdpe.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::SUCCESS; }
	if (pdpe.get_bit(EntryControlBit::PageSize)) {
		pdpe.set_bit(EntryControlBit::PagePresent, false);
		if (table_empty(pdp)) {
			m_pml4[pml4_index(vaddr)].set_bit(EntryControlBit::PagePresent, false);
			MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pdp)));
		}
		invalidate_page(vaddr);
		return MdOS::Result::SUCCESS;
	}

	Entry *pd = (Entry *) MDOS_PHYS_TO_VIRT(pdpe.get_addr());
	Entry &pde = pd[pd_index(vaddr)];
	if (!pde.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::SUCCESS; }
	if (pde.get_bit(EntryControlBit::PageSize)) {
		pde.set_bit(EntryControlBit::PagePresent, false);
		if (table_empty(pd)) {
			pdpe.set_bit(EntryControlBit::PagePresent, false);
			MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pd)));
			if (table_empty(pdp)) {
				m_pml4[pml4_index(vaddr)].set_bit(EntryControlBit::PagePresent, false);
				MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pdp)));
			}
		}
		invalidate_page(vaddr);
		return MdOS::Result::SUCCESS;
	}

	Entry *pt = (Entry *) MDOS_PHYS_TO_VIRT(pd[pd_index(vaddr)].get_addr());
	Entry &pte = pt[pt_index(vaddr)];
	if (!pte.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::SUCCESS; }
	pte.set_bit(EntryControlBit::PagePresent, false);
	if (table_empty(pt)) {
		pd[pd_index(vaddr)].set_bit(EntryControlBit::PagePresent, false);
		MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pt)));
		if (table_empty(pd)) {
			pdpe.set_bit(EntryControlBit::PagePresent, false);
			MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pd)));
		}
		if (table_empty(pdp)) {
			m_pml4[pml4_index(vaddr)].set_bit(EntryControlBit::PagePresent, false);
			MdOS::Memory::PMM::free_page(uintptr_t(MDOS_VIRT_TO_PHYS(pdp)));
		}
	}
	invalidate_page(vaddr);
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_4KiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																		   EntryFlagBits flags) {
	kassert((paddr % MdOS::Memory::Paging::pageSize4KiB) == 0);
	kassert((vaddr % MdOS::Memory::Paging::pageSize4KiB) == 0);
	Entry *pdp = get_entry(m_pml4, pml4_index(vaddr), EntryType::PML4E);
	if (pdp[pdp_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MdOS::Result::INVALID_PARAMETER; }
	Entry *pd = get_entry(pdp, pdp_index(vaddr), EntryType::PDPE);
	if (pd[pd_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MdOS::Result::INVALID_PARAMETER; }
	Entry *pt = get_entry(pd, pd_index(vaddr), EntryType::PDE);

	Entry ent = {0ULL};
	ent.set_type(EntryType::PTE);
	ent.set_addr(paddr);
	ent.set_bit(EntryControlBit::PagePresent, true);
	if (flags & ReadWrite) { ent.set_bit(EntryControlBit::ReadWrite, true); }
	if (flags & UserAccessible) { ent.set_bit(EntryControlBit::UserAccessible, true); }
	if (flags & NoExecute) { ent.set_bit(EntryControlBit::NoExecute, true); }
	if (flags & WriteThrough) { ent.set_bit(EntryControlBit::WriteThrough, true); }
	if (flags & CacheDisable) { ent.set_bit(EntryControlBit::CacheDisable, true); }
	if (flags & Global) { ent.set_bit(EntryControlBit::Global, true); }
	if (flags & PAT) { ent.set_bit(EntryControlBit::PageAttributeTable, true); }
	pt[pt_index(vaddr)] = ent;

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_2MiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																		   EntryFlagBits flags) {
	kassert((paddr % MdOS::Memory::Paging::pageSize2MiB) == 0);
	kassert((vaddr % MdOS::Memory::Paging::pageSize2MiB) == 0);

	Entry *pdp = get_entry(m_pml4, pml4_index(vaddr), EntryType::PML4E);
	if (pdp[pdp_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MdOS::Result::INVALID_PARAMETER; }
	Entry *pd = get_entry(pdp, pdp_index(vaddr), EntryType::PDPE);

	Entry ent = {0ULL};
	ent.set_type(EntryType::PDE);
	ent.set_addr(paddr);
	ent.set_bit(EntryControlBit::PagePresent, true);
	ent.set_bit(EntryControlBit::PageSize, true);
	if (flags & ReadWrite) { ent.set_bit(EntryControlBit::ReadWrite, true); }
	if (flags & UserAccessible) { ent.set_bit(EntryControlBit::UserAccessible, true); }
	if (flags & NoExecute) { ent.set_bit(EntryControlBit::NoExecute, true); }
	if (flags & WriteThrough) { ent.set_bit(EntryControlBit::WriteThrough, true); }
	if (flags & CacheDisable) { ent.set_bit(EntryControlBit::CacheDisable, true); }
	if (flags & Global) { ent.set_bit(EntryControlBit::Global, true); }
	if (flags & PAT) { ent.set_bit(EntryControlBit::PageAttributeTable, true); }
	pd[pd_index(vaddr)] = ent;

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_1GiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																		   EntryFlagBits flags) {
	kassert((paddr % MdOS::Memory::Paging::pageSize1GiB) == 0);
	kassert((vaddr % MdOS::Memory::Paging::pageSize1GiB) == 0);

	Entry *pdp = get_entry(m_pml4, pml4_index(vaddr), EntryType::PML4E);

	Entry ent = {0ULL};
	ent.set_type(EntryType::PDPE);
	ent.set_addr(paddr);
	ent.set_bit(EntryControlBit::PagePresent, true);
	ent.set_bit(EntryControlBit::PageSize, true);
	if (flags & ReadWrite) { ent.set_bit(EntryControlBit::ReadWrite, true); }
	if (flags & UserAccessible) { ent.set_bit(EntryControlBit::UserAccessible, true); }
	if (flags & NoExecute) { ent.set_bit(EntryControlBit::NoExecute, true); }
	if (flags & WriteThrough) { ent.set_bit(EntryControlBit::WriteThrough, true); }
	if (flags & CacheDisable) { ent.set_bit(EntryControlBit::CacheDisable, true); }
	if (flags & Global) { ent.set_bit(EntryControlBit::Global, true); }
	if (flags & PAT) { ent.set_bit(EntryControlBit::PageAttributeTable, true); }
	pdp[pdp_index(vaddr)] = ent;

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::swap_attributes(VirtualAddress vaddr,
																			 EntryFlagBits newFlags) {
	Entry &pml4e = m_pml4[pml4_index(vaddr)];
	if (!pml4e.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::PAGE_NOT_PRESENT; }
	if (!is_canonical_pml4(vaddr)) { return MdOS::Result::INVALID_PARAMETER; }

	Entry *pdp = (Entry *) MDOS_PHYS_TO_VIRT(pml4e.get_addr());
	Entry &pdpe = pdp[pdp_index(vaddr)];
	if (!pdpe.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::PAGE_NOT_PRESENT; }
	if (pdpe.get_bit(EntryControlBit::PageSize)) {
		pdpe.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
		pdpe.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
		pdpe.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
		pdpe.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
		pdpe.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
		pdpe.set_bit(EntryControlBit::Global, newFlags & Global);
		pdpe.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
		invalidate_page(vaddr);
		return MdOS::Result::SUCCESS;
	}

	Entry *pd = (Entry *) MDOS_PHYS_TO_VIRT(pdpe.get_addr());
	Entry &pde = pd[pd_index(vaddr)];
	if (!pde.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::PAGE_NOT_PRESENT; }
	if (pde.get_bit(EntryControlBit::PageSize)) {
		pde.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
		pde.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
		pde.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
		pde.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
		pde.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
		pde.set_bit(EntryControlBit::Global, newFlags & Global);
		pde.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
		invalidate_page(vaddr);
		return MdOS::Result::SUCCESS;
	}

	Entry *pt = (Entry *) MDOS_PHYS_TO_VIRT(pde.get_addr());
	Entry &pte = pt[pt_index(vaddr)];
	if (!pte.get_bit(EntryControlBit::PagePresent)) { return MdOS::Result::PAGE_NOT_PRESENT; }
	pte.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
	pte.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
	pte.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
	pte.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
	pte.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
	pte.set_bit(EntryControlBit::Global, newFlags & Global);
	pte.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
	invalidate_page(vaddr);

	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_range(PhysicalAddress paddrBase,
																	   VirtualAddress vaddrBase, size_t numPages,
																	   EntryFlagBits flags) {
	kassert((paddrBase % MdOS::Memory::Paging::pageSize4KiB) == 0);
	kassert((vaddrBase % MdOS::Memory::Paging::pageSize4KiB) == 0);
	for (size_t i = 0; i < numPages; i++) {
		MdOS::Result res = map_page(paddrBase + (i * MdOS::Memory::Paging::pageSize4KiB),
									vaddrBase + (i * MdOS::Memory::Paging::pageSize4KiB), flags);
		if (res != MdOS::Result::SUCCESS) {
			unmap_range(vaddrBase, i);
			return res;
		}
	}
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::map_smart_range(PhysicalAddress paddrBase,
																			 VirtualAddress vaddrBase, size_t size,
																			 EntryFlagBits flags) {
	kassert((paddrBase % MdOS::Memory::Paging::pageSize4KiB) == 0);
	kassert((vaddrBase % MdOS::Memory::Paging::pageSize4KiB) == 0);

	PhysicalAddress paddr = paddrBase;
	VirtualAddress vaddr = vaddrBase;
	size_t rem = size;

	while (rem > 0) {
		MdOS::Result res;
		if ((paddr % pageSize1GiB) == 0 && (vaddr % pageSize1GiB) == 0 && rem > pageSize1GiB) {
			res = map_1GiB_page(paddr, vaddr, flags);
			paddr += pageSize1GiB;
			vaddr += pageSize1GiB;
			rem -= pageSize1GiB;
		} else if ((paddr % pageSize2MiB) == 0 && (vaddr % pageSize2MiB) == 0 && rem > pageSize2MiB) {
			res = map_2MiB_page(paddr, vaddr, flags);
			paddr += pageSize2MiB;
			vaddr += pageSize2MiB;
			rem -= pageSize2MiB;
		} else {
			res = map_4KiB_page(paddr, vaddr, flags);
			paddr += pageSize4KiB;
			vaddr += pageSize4KiB;
			rem -= pageSize4KiB;
		}
		if (res != MdOS::Result::SUCCESS) { return res; }
		// TODO: implement unmap_range function that works with 2MiB and 1GiB pages. Unmap pages on failure
	}
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::VirtualMemoryManagerPML4::unmap_range(VirtualAddress vaddrBase, size_t numPages) {
	kassert((vaddrBase % MdOS::Memory::Paging::pageSize4KiB) == 0);
	for (size_t i = 0; i < numPages; i++) {
		MdOS::Result res = unmap_page(vaddrBase + (i * MdOS::Memory::Paging::pageSize4KiB));
		if (res != MdOS::Result::SUCCESS) { PANIC("Failed to unmap memory range", MEMORY_ERROR); }
	}
	return MdOS::Result::SUCCESS;
}

MdOS::Result MdOS::Memory::Paging::map_kernel(SectionInfo *sections, size_t sectionInfoCount, MemMap *memMap,
											  BootstrapMemoryRegion bootHeap, GOPFramebuffer *framebuffer,
											  VirtualMemoryManagerPML4 *vmm) {
	MdOS::Result res;
	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));
		res = vmm->map_smart_range(entry->paddr, MDOS_PHYS_TO_VIRT(entry->paddr), entry->pageCount * pageSize4KiB, ReadWrite);
		if (res != MdOS::Result::SUCCESS) { return res; }
	}
	res = vmm->map_range(PhysicalAddress(bootHeap.basePaddr), VirtualAddress(bootHeap.baseAddr),
						 bootHeap.size / pageSize4KiB, ReadWrite);
	if (res != MdOS::Result::SUCCESS) { return res; }
	res = vmm->map_range(PhysicalAddress(framebuffer->bufferBase), VirtualAddress(framebuffer->bufferBase),
						 framebuffer->bufferSize / 0x1000, ReadWrite);
	if (res != MdOS::Result::SUCCESS) { return res; }
	for (size_t i = 0; i < sectionInfoCount; i++) {
		SectionInfo *section = (SectionInfo *) (uintptr_t(sections) + i * sizeof(SectionInfo));
		res = vmm->map_range(section->paddr, section->vaddr, section->pageCount, ReadWrite);
		if (res != MdOS::Result::SUCCESS) { return res; }
	}

	return MdOS::Result::SUCCESS;
}