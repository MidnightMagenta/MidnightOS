#include <IO/debug_print.h>
#include <error/panic.h>
#include <k_utils/utils.hpp>
#include <libk/assert.h>
#include <memory/paging.hpp>
#include <memory/paging_index_helper.hpp>
#include <memory/pmm.hpp>

#define OFFSET_MASK_1GiB 0x3FFFFFFF
#define OFFSET_MASK_2MiB 0x1FFFFF
#define OFFSET_MASK_4KiB 0xFFF

MdOS::mem::virt::VirtualMemoryManagerPML4 *MdOS::mem::virt::VirtualMemoryManagerPML4::m_boundVMM = nullptr;

Result MdOS::mem::virt::VirtualMemoryManagerPML4::init() {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);

	if (m_pml4 != nullptr) { return MDOS_ALREADY_INITIALIZED; }
	MdOS::mem::phys::PhysicalMemoryAllocation allocation;
	Result res = MdOS::mem::phys::alloc_pages(&allocation);
	if (res != MDOS_SUCCESS) {
		DEBUG_LOG_VB1("Failed to allocate PML4 with: %u", uint32_t(res));
		return res;
	}
	m_pml4 = (Entry *) (MDOS_PHYS_TO_VIRT(allocation.base));
	memset((void *) m_pml4, 0x0, MdOS::mem::virt::pageSize4KiB);
	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::init(Entry *pml4) {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);
	if (pml4 == nullptr) { return MDOS_INIT_FAILURE; }
	m_pml4 = pml4;
	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::init(VirtualMemoryManagerPML4 *vmm __attribute__((unused))) {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);
	Result res = this->init();
	if (res != MDOS_SUCCESS || m_pml4 == nullptr) { return res; }
	//memcpy(m_pml4, vmm->get_pml4(), pageTableSize);
	//TODO: implement copy initialization
	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

bool MdOS::mem::virt::VirtualMemoryManagerPML4::table_empty(Entry *table) {
	LOG_FUNC_ENTRY;
	for (size_t i = 0; i < 512; i++) {
		if (table[i].get_bit(EntryControlBit::PagePresent)) { return false; }
	}
	LOG_FUNC_EXIT;
	return true;
}

void MdOS::mem::virt::VirtualMemoryManagerPML4::free_table_if_empty(Entry *table, Entry &entry) {
	LOG_FUNC_ENTRY;
	if (table_empty(table)) {
		entry.set_bit(EntryControlBit::PagePresent, false);
		MdOS::mem::phys::free_page(uintptr_t(table));
	}
	LOG_FUNC_EXIT;
}

MdOS::mem::virt::Entry *MdOS::mem::virt::VirtualMemoryManagerPML4::get_entry(Entry *table, size_t index,
																			 EntryType type) {
	LOG_FUNC_ENTRY;
	Entry ent = table[index];
	if (!ent.get_bit(EntryControlBit::PagePresent)) {
		ent = {0ULL};
		ent.set_type(type);
		ent.set_bit(EntryControlBit::PagePresent, true);
		ent.set_bit(EntryControlBit::ReadWrite, true);
		ent.set_bit(EntryControlBit::UserAccessible, true);
		ent.set_addr(MdOS::mem::phys::alloc_page());
		kassert(ent.get_addr() != 0);
		memset((void *) (MDOS_PHYS_TO_VIRT(ent.get_addr())), 0x0, MdOS::mem::virt::pageSize4KiB);
		table[index] = ent;
	}
	kassert(ent.get_addr() != 0);
	LOG_FUNC_EXIT;
	return (Entry *) MDOS_PHYS_TO_VIRT(ent.get_addr());
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::map_4KiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																EntryFlagBits flags) {
	LOG_FUNC_ENTRY;
	kassert((paddr % MdOS::mem::virt::pageSize4KiB) == 0);
	kassert((vaddr % MdOS::mem::virt::pageSize4KiB) == 0);
	Entry *pdp = get_entry(m_pml4, pml4_index(vaddr), EntryType::PML4E);
	if (pdp[pdp_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MDOS_INVALID_PARAMETER; }
	Entry *pd = get_entry(pdp, pdp_index(vaddr), EntryType::PDPE);
	if (pd[pd_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MDOS_INVALID_PARAMETER; }
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

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::map_2MiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																EntryFlagBits flags) {
	LOG_FUNC_ENTRY;
	kassert((paddr % MdOS::mem::virt::pageSize2MiB) == 0);
	kassert((vaddr % MdOS::mem::virt::pageSize2MiB) == 0);

	Entry *pdp = get_entry(m_pml4, pml4_index(vaddr), EntryType::PML4E);
	if (pdp[pdp_index(vaddr)].get_bit(EntryControlBit::PageSize)) { return MDOS_INVALID_PARAMETER; }
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

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::map_1GiB_page(PhysicalAddress paddr, VirtualAddress vaddr,
																EntryFlagBits flags) {
	LOG_FUNC_ENTRY;
	kassert((paddr % MdOS::mem::virt::pageSize1GiB) == 0);
	kassert((vaddr % MdOS::mem::virt::pageSize1GiB) == 0);

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

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::swap_attributes(VirtualAddress vaddr, EntryFlagBits newFlags) {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);

	ALLOC_LOG("Swapping attributes for vaddr 0x%lx to 0x%x", vaddr, newFlags);

	if (m_pml4 == nullptr) {
		Result res = init();
		if (res != MDOS_SUCCESS) { return res; }
	}
	kassert(m_pml4 != nullptr);
	Entry &pml4e = m_pml4[pml4_index(vaddr)];
	if (!pml4e.get_bit(EntryControlBit::PagePresent)) { return MDOS_PAGE_NOT_PRESENT; }
	if (!is_canonical_pml4(vaddr)) { return MDOS_INVALID_PARAMETER; }

	Entry *pdp = (Entry *) MDOS_PHYS_TO_VIRT(pml4e.get_addr());
	Entry &pdpe = pdp[pdp_index(vaddr)];
	if (!pdpe.get_bit(EntryControlBit::PagePresent)) { return MDOS_PAGE_NOT_PRESENT; }
	if (pdpe.get_bit(EntryControlBit::PageSize)) {
		pdpe.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
		pdpe.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
		pdpe.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
		pdpe.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
		pdpe.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
		pdpe.set_bit(EntryControlBit::Global, newFlags & Global);
		pdpe.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
		invalidate_page(vaddr);
		return MDOS_SUCCESS;
	}

	Entry *pd = (Entry *) MDOS_PHYS_TO_VIRT(pdpe.get_addr());
	Entry &pde = pd[pd_index(vaddr)];
	if (!pde.get_bit(EntryControlBit::PagePresent)) { return MDOS_PAGE_NOT_PRESENT; }
	if (pde.get_bit(EntryControlBit::PageSize)) {
		pde.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
		pde.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
		pde.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
		pde.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
		pde.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
		pde.set_bit(EntryControlBit::Global, newFlags & Global);
		pde.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
		invalidate_page(vaddr);
		return MDOS_SUCCESS;
	}

	Entry *pt = (Entry *) MDOS_PHYS_TO_VIRT(pde.get_addr());
	Entry &pte = pt[pt_index(vaddr)];
	if (!pte.get_bit(EntryControlBit::PagePresent)) { return MDOS_PAGE_NOT_PRESENT; }
	pte.set_bit(EntryControlBit::ReadWrite, newFlags & ReadWrite);
	pte.set_bit(EntryControlBit::UserAccessible, newFlags & UserAccessible);
	pte.set_bit(EntryControlBit::NoExecute, newFlags & NoExecute);
	pte.set_bit(EntryControlBit::WriteThrough, newFlags & WriteThrough);
	pte.set_bit(EntryControlBit::CacheDisable, newFlags & CacheDisable);
	pte.set_bit(EntryControlBit::Global, newFlags & Global);
	pte.set_bit(EntryControlBit::PageAttributeTable, newFlags & PAT);
	invalidate_page(vaddr);

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::map_range(PhysicalAddress paddrBase, VirtualAddress vaddrBase,
															size_t size, EntryFlagBits flags) {
	LOG_FUNC_ENTRY;
	ALLOC_LOG("Mapping memory range 0x%lx - 0x%lx to vaddr 0x%lx with flags 0x%x", paddrBase, paddrBase + size,
			  vaddrBase, flags);
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);

	if (m_pml4 == nullptr) {
		Result res = init();
		if (res != MDOS_SUCCESS) { return res; }
	}
	kassert(m_pml4 != nullptr);
	kassert((paddrBase % MdOS::mem::virt::pageSize4KiB) == 0);
	kassert((vaddrBase % MdOS::mem::virt::pageSize4KiB) == 0);
	kassert((size % MdOS::mem::virt::pageSize4KiB) == 0);

	PhysicalAddress paddr = paddrBase;
	VirtualAddress vaddr = vaddrBase;
	size_t rem = size;

	while (rem > 0) {
		Result res;
		if ((paddr % pageSize1GiB) == 0 && (vaddr % pageSize1GiB) == 0 && rem > pageSize1GiB) {
			res = map_1GiB_page(paddr, vaddr, flags);
			invalidate_page(vaddr);
			paddr += pageSize1GiB;
			vaddr += pageSize1GiB;
			if (rem < pageSize1GiB) return MDOS_OVERFLOW;
			rem -= pageSize1GiB;
		} else if ((paddr % pageSize2MiB) == 0 && (vaddr % pageSize2MiB) == 0 && rem > pageSize2MiB) {
			res = map_2MiB_page(paddr, vaddr, flags);
			invalidate_page(vaddr);
			paddr += pageSize2MiB;
			vaddr += pageSize2MiB;
			if (rem < pageSize2MiB) return MDOS_OVERFLOW;
			rem -= pageSize2MiB;
		} else {
			res = map_4KiB_page(paddr, vaddr, flags);
			invalidate_page(vaddr);
			paddr += pageSize4KiB;
			vaddr += pageSize4KiB;
			if (rem < pageSize4KiB) return MDOS_OVERFLOW;
			rem -= pageSize4KiB;
		}
		if (res != MDOS_SUCCESS) {
			if (unmap_range(vaddrBase, size - rem) != MDOS_SUCCESS) {
				PANIC("Failed to unmap memory range", MDOS_PANIC_MEMORY_ERROR);
			}
			return res;
		}
	}

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

Result MdOS::mem::virt::VirtualMemoryManagerPML4::unmap_range(VirtualAddress vaddrBase, size_t size) {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);

	ALLOC_LOG("Unmapping virtual address range 0x%lx - 0x%lx", vaddrBase, vaddrBase + size);

	if (m_pml4 == nullptr) {
		Result res = init();
		if (res != MDOS_SUCCESS) { return res; }
	}
	kassert(m_pml4 != nullptr);
	kassert((vaddrBase % pageSize4KiB) == 0);
	kassert((size % pageSize4KiB) == 0);

	VirtualAddress vaddr = vaddrBase;
	size_t rem = size;
	while (rem > 0) {
		if (rem < pageSize4KiB) { return MDOS_OVERFLOW; }
		kassert(is_canonical_pml4(vaddr));

		Entry &pml4e = m_pml4[pml4_index(vaddr)];
		if (!pml4e.get_bit(EntryControlBit::PagePresent)) {
			if (rem < pageSizePML4) { return MDOS_OVERFLOW; }
			vaddr += pageSizePML4;
			rem -= pageSizePML4;
			continue;
		}

		Entry *pdp = (Entry *) MDOS_PHYS_TO_VIRT(pml4e.get_addr());
		Entry &pdpe = pdp[pdp_index(vaddr)];
		if (!pdpe.get_bit(EntryControlBit::PagePresent)) {
			if (rem < pageSize1GiB) { return MDOS_OVERFLOW; }
			vaddr += pageSize1GiB;
			rem -= pageSize1GiB;
			continue;
		}
		if (pdpe.get_bit(EntryControlBit::PageSize)) {
			pdpe.set_bit(EntryControlBit::PagePresent, false);
			invalidate_page(vaddr);
			if (rem < pageSize1GiB) { return MDOS_OVERFLOW; }
			vaddr += pageSize1GiB;
			rem -= pageSize1GiB;
			free_table_if_empty(pdp, pml4e);
			continue;
		}

		Entry *pd = (Entry *) MDOS_PHYS_TO_VIRT(pdpe.get_addr());
		Entry &pde = pd[pd_index(vaddr)];
		if (!pde.get_bit(EntryControlBit::PagePresent)) {
			if (rem < pageSize2MiB) { return MDOS_OVERFLOW; }
			vaddr += pageSize2MiB;
			rem -= pageSize2MiB;
			continue;
		}
		if (pde.get_bit(EntryControlBit::PageSize)) {
			pde.set_bit(EntryControlBit::PagePresent, false);
			invalidate_page(vaddr);
			if (rem < pageSize2MiB) { return MDOS_OVERFLOW; }
			vaddr += pageSize2MiB;
			rem -= pageSize2MiB;
			free_table_if_empty(pd, pdpe);
			free_table_if_empty(pdp, pml4e);
			continue;
		}

		Entry *pt = (Entry *) MDOS_PHYS_TO_VIRT(pde.get_addr());
		Entry &pte = pt[pt_index(vaddr)];
		if (!pte.get_bit(EntryControlBit::PagePresent)) {
			vaddr += pageSize4KiB;
			rem -= pageSize4KiB;
			continue;
		}

		pte.set_bit(EntryControlBit::PagePresent, false);
		invalidate_page(vaddr);
		vaddr += pageSize4KiB;
		rem -= pageSize4KiB;
		free_table_if_empty(pt, pde);
		free_table_if_empty(pd, pdpe);
		free_table_if_empty(pdp, pml4e);
		continue;
	}

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

PhysicalAddress MdOS::mem::virt::VirtualMemoryManagerPML4::query_paddr(VirtualAddress vaddr) {
	LOG_FUNC_ENTRY;
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);

	if (m_pml4 == nullptr) { return 0; }

	Entry pml4e = m_pml4[pml4_index(vaddr)];
	if (!pml4e.get_bit(EntryControlBit::PagePresent)) { return 0; }

	Entry *pdp = (Entry *) pml4e.get_addr();
	Entry pdpe = pdp[pdp_index(vaddr)];
	if (!pdpe.get_bit(EntryControlBit::PagePresent)) { return 0; }
	if (pdpe.get_bit(EntryControlBit::PageSize)) {
		LOG_FUNC_EXIT;
		return pdpe.get_addr() + (vaddr & OFFSET_MASK_1GiB);
	}

	Entry *pd = (Entry *) pdpe.get_addr();
	Entry pde = pd[pd_index(vaddr)];
	if (!pde.get_bit(EntryControlBit::PagePresent)) { return 0; }
	if (pde.get_bit(EntryControlBit::PageSize)) {
		LOG_FUNC_EXIT;
		return pde.get_addr() + (vaddr & OFFSET_MASK_2MiB);
	}

	Entry *pt = (Entry *) pde.get_addr();
	Entry pte = pt[pt_index(vaddr)];
	if (!pte.get_bit(EntryControlBit::PagePresent)) { return 0; }

	LOG_FUNC_EXIT;
	return pte.get_addr() + (vaddr & OFFSET_MASK_4KiB);
}

Result MdOS::mem::virt::map_kernel(SectionInfo *sections, size_t sectionInfoCount, MemMap *memMap,
								   BootstrapMemoryRegion bootHeap, GOPFramebuffer *framebuffer,
								   VirtualMemoryManagerPML4 *vmm) {
	LOG_FUNC_ENTRY;
	Result res;
	// build the direct map
	for (size_t i = 0; i < memMap->size / memMap->descriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *entry =
				(EFI_MEMORY_DESCRIPTOR *) ((uintptr_t) memMap->map + (i * memMap->descriptorSize));
		res = vmm->map_range(entry->paddr, MDOS_PHYS_TO_VIRT(entry->paddr), entry->pageCount * pageSize4KiB, ReadWrite);
		if (res != MDOS_SUCCESS) { return res; }
	}

	// map the boot heap
	res = vmm->map_range(PhysicalAddress(bootHeap.basePaddr), VirtualAddress(bootHeap.baseAddr), bootHeap.size,
						 ReadWrite);
	if (res != MDOS_SUCCESS) { return res; }

	// map the GOP framebuffer
	res = vmm->map_range(PhysicalAddress(framebuffer->bufferBase), VirtualAddress(framebuffer->bufferBase),
						 framebuffer->bufferSize, ReadWrite);
	if (res != MDOS_SUCCESS) { return res; }

	// mape the kernel sections
	for (size_t i = 0; i < sectionInfoCount; i++) {
		SectionInfo *section = (SectionInfo *) (uintptr_t(sections) + i * sizeof(SectionInfo));
		res = vmm->map_range(section->paddr, section->vaddr, section->pageCount * pageSize4KiB, ReadWrite);
		if (res != MDOS_SUCCESS) { return res; }
	}

	LOG_FUNC_EXIT;
	return MDOS_SUCCESS;
}

#ifdef _DEBUG

void MdOS::mem::virt::VirtualMemoryManagerPML4::print_entry(Entry entry) {
	kprint("raw: 0x%lx | addr: 0x%lx [%s%s%s%s%s%s%s]", entry, entry.get_addr(),
		   (entry.get_bit(EntryControlBit::NoExecute)) ? "NX " : "\0",
		   (entry.get_bit(EntryControlBit::Accessed)) ? "A " : "\0",
		   (entry.get_bit(EntryControlBit::CacheDisable)) ? "PCD " : "\0",
		   (entry.get_bit(EntryControlBit::WriteThrough)) ? "PWT " : "\0",
		   (entry.get_bit(EntryControlBit::UserAccessible)) ? "U/S " : "SO ",
		   (entry.get_bit(EntryControlBit::ReadWrite)) ? "R/W " : "RO ",
		   (entry.get_bit(EntryControlBit::PagePresent)) ? "P" : "NP");
}

void MdOS::mem::virt::VirtualMemoryManagerPML4::dump_pml4(bool presentOnly) {
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);
	if (m_pml4 == nullptr) {
		Result res = init();
		if (res != MDOS_SUCCESS) { return; }
	}
	kassert(m_pml4 != nullptr);
	DEBUG_LOG_VB1("PML4 dump:\n");
	for (size_t i = 0; i < pageTableSize / sizeof(PageEntry); i++) {
		if (presentOnly) {
			if (m_pml4[i].get_bit(EntryControlBit::PagePresent)) {
				kprint("\tEntry: %lu | ", i);
				print_entry(m_pml4[i]);
				kprint("\n");
			}
		} else {
			kprint("\tEntry: %lu | ", i);
			print_entry(m_pml4[i]);
			kprint("\n");
		}
	}
}

void MdOS::mem::virt::VirtualMemoryManagerPML4::dump_translation_hierarchy(VirtualAddress vaddr) {
	MdOS::thread::LockGuard<MdOS::thread::Spinlock> lock(&m_lock);
	if (m_pml4 == nullptr) {
		Result res = init();
		if (res != MDOS_SUCCESS) { return; }
	}
	kassert(m_pml4 != nullptr);
	Entry pml4e = m_pml4[pml4_index(vaddr)];

	Entry pdpe = {0ULL};
	if (pml4e.get_bit(EntryControlBit::PagePresent)) {
		pdpe = ((Entry *) (MDOS_PHYS_TO_VIRT(pml4e.get_addr())))[pdp_index(vaddr)];
	}
	Entry pde = {0ULL};
	if (pdpe.get_bit(EntryControlBit::PagePresent) && !pdpe.get_bit(EntryControlBit::PageSize)) {
		pde = ((Entry *) (MDOS_PHYS_TO_VIRT(pdpe.get_addr())))[pd_index(vaddr)];
	}
	Entry pte = {0ULL};
	if (pde.get_bit(EntryControlBit::PagePresent) && !pdpe.get_bit(EntryControlBit::PageSize)) {
		pte = ((Entry *) (MDOS_PHYS_TO_VIRT(pde.get_addr())))[pt_index(vaddr)];
	}

	DEBUG_LOG_VB1("Translation hierarchy for address: 0x%lx\n", vaddr);
	kprint("\tPML4E: ");
	print_entry(pml4e);
	kprint("\n");
	kprint("\tPDPE:  ");
	print_entry(pdpe);
	kprint("\n");
	kprint("\tPDE:   ");
	print_entry(pde);
	kprint("\n");
	kprint("\tPTE:   ");
	print_entry(pte);
	kprint("\n");
	kprint("\tOffset: 0x%x\n", offset(vaddr));
}

#endif