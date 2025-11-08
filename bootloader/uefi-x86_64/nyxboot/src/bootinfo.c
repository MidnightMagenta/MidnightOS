#include "../include/bootinfo.h"
#include "efilib.h"

static bi_memtype memtype_efi_to_bi(EFI_MEMORY_TYPE efiType) {
    switch (efiType) {
        case EfiReservedMemoryType:
            return BI_MEMTYPE_RESERVED;
        case EfiLoaderCode:
            return BI_MEMTYPE_RECLAIMABLE;
        case EfiLoaderData:
            return BI_MEMTYPE_RECLAIMABLE;
        case EfiBootServicesCode:
            return BI_MEMTYPE_RECLAIMABLE;
        case EfiBootServicesData:
            return BI_MEMTYPE_RECLAIMABLE;
        case EfiRuntimeServicesCode:
            return BI_MEMTYPE_EFI_RT_CODE;
        case EfiRuntimeServicesData:
            return BI_MEMTYPE_EFI_RT_DATA;
        case EfiConventionalMemory:
            return BI_MEMTYPE_USABLE;
        case EfiUnusableMemory:
            return BI_MEMTYPE_UNUSABLE;
        case EfiACPIReclaimMemory:
            return BI_MEMTYPE_ACPI_RECLAIMABLE;
        case EfiACPIMemoryNVS:
            return BI_MEMTYPE_ACPI_NVS;
        case EfiMemoryMappedIO:
            return BI_MEMTYPE_MMIO;
        case EfiMemoryMappedIOPortSpace:
            return BI_MEMTYPE_MMIO_PORT_SPACE;
        case EfiPalCode:
            return BI_MEMTYPE_RESERVED;
        case EfiPersistentMemory:
            return BI_MEMTYPE_PERSISTENT_MEM;
        case EfiUnacceptedMemoryType:
            return BI_MEMTYPE_UNUSABLE;
        case EfiMaxMemoryType:
            return BI_MEMTYPE_UNKNOWN;
        default:
            return BI_MEMTYPE_UNKNOWN;
    }
}

#define BI_MAP_DESCRIPTOR_EXTRA 8192

static EFI_STATUS bi_build_memmap(bi_memmap_t **memmap) {
    EFI_STATUS res = gBS->AllocatePool(EfiLoaderData, sizeof(bi_memmap_t), (void **) memmap);
    if (EFI_ERROR(res)) { return res; }

    SetMem(*memmap, sizeof(bi_memmap_t), 0);

    (*memmap)->version         = BI_VERSION_1;
    (*memmap)->descriptorSize  = sizeof(bi_memdesc_t);
    (*memmap)->descriptorCount = 256;
    (*memmap)->bufferSize      = ((*memmap)->descriptorCount * (*memmap)->descriptorSize) + BI_MAP_DESCRIPTOR_EXTRA;

    bi_memdesc_t *descriptors = NULL;
    res                       = gBS->AllocatePool(EfiLoaderData, (*memmap)->bufferSize, (void **) &descriptors);
    if (EFI_ERROR(res)) {
        gBS->FreePool(*memmap);
        *memmap = NULL;
        return res;
    }

    SetMem(descriptors, (*memmap)->bufferSize, 0);
    (*memmap)->pDescriptors = (uint64_t) (UINTN) descriptors;
    return EFI_SUCCESS;
}

static EFI_STATUS bi_build_kernelmap(const elf_loadinfo_t *const elfMap, bi_kernelmap_t **kernelMap) {
    EFI_STATUS res = gBS->AllocatePool(EfiLoaderData, sizeof(bi_kernelmap_t), (void **) kernelMap);
    if (EFI_ERROR(res)) { return res; }

    SetMem(*kernelMap, sizeof(bi_kernelmap_t), 0);

    (*kernelMap)->version         = BI_VERSION_1;
    (*kernelMap)->descriptorSize  = sizeof(bi_kernelmapdesc_t);
    (*kernelMap)->descriptorCount = elfMap->sectionCount;

    UINTN totalBytes = (*kernelMap)->descriptorSize * (*kernelMap)->descriptorCount;

    bi_kernelmapdesc_t *descriptors = NULL;
    res                             = gBS->AllocatePool(EfiLoaderData, totalBytes, (void **) &descriptors);
    if (EFI_ERROR(res)) {
        gBS->FreePool(*kernelMap);
        *kernelMap = NULL;
        return res;
    }
    SetMem(descriptors, totalBytes, 0);

    for (UINTN i = 0; i < elfMap->sectionCount; i++) {
        const elf_sectioninfo_t *section =
                (const elf_sectioninfo_t *) ((const char *) elfMap->sections + i * sizeof(elf_sectioninfo_t));
        bi_kernelmapdesc_t *desc = (bi_kernelmapdesc_t *) ((char *) descriptors + i * sizeof(bi_kernelmapdesc_t));

        desc->paddr     = section->phys;
        desc->vaddr     = section->reqVirt;
        desc->pageCount = section->pageCount;
        desc->flags     = 0;// TODO: add flag handling
    }

    (*kernelMap)->pDescriptors = (uint64_t) (UINTN) descriptors;
    return EFI_SUCCESS;
}

EFI_STATUS bi_build_bootinfo(bi_bootinfo_createinfo *createInfo, bi_bootinfo_t **bootInfo) {
    EFI_STATUS res = gBS->AllocatePool(EfiLoaderCode, sizeof(bi_bootinfo_t), (void **) bootInfo);
    if (EFI_ERROR(res)) { return res; }

    SetMem(*bootInfo, sizeof(bi_bootinfo_t), 0);

    (*bootInfo)->head.version       = BI_VERSION_1;
    (*bootInfo)->head.structureType = BI_STRUCTURE_TYPE_BOOT_INFO;
    (*bootInfo)->head.pNext         = (bi_physaddress_t) NULL;
    (*bootInfo)->magic[0]           = BI_MAGIC0;
    (*bootInfo)->magic[1]           = BI_MAGIC1;
    (*bootInfo)->magic[2]           = BI_MAGIC2;
    (*bootInfo)->magic[3]           = BI_MAGIC3;
    (*bootInfo)->selfSize           = sizeof(bi_bootinfo_t);
    (*bootInfo)->flags              = BI_BOOTFLAGS_EFI_BOOT;

    // Build sub-structures
    res = bi_build_memmap((bi_memmap_t **) &(*bootInfo)->pMemMap);
    if (EFI_ERROR(res)) { goto fail_bootinfo; }

    res = bi_build_kernelmap(createInfo->elfSections, (bi_kernelmap_t **) &(*bootInfo)->pKernelMap);
    if (EFI_ERROR(res)) { goto fail_memmap; }

    return EFI_SUCCESS;

fail_memmap:
    if ((*bootInfo)->pMemMap) {
        void        *ptr = (void *) (UINTN) (*bootInfo)->pMemMap;
        bi_memmap_t *mm  = (bi_memmap_t *) ptr;
        if (mm->pDescriptors) { gBS->FreePool((void *) (UINTN) mm->pDescriptors); }
        gBS->FreePool(mm);
    }
fail_bootinfo:
    gBS->FreePool(*bootInfo);
    *bootInfo = NULL;
    return res;
}

EFI_STATUS bi_update_memmap(const efi_memmap_t *const efiMap, bi_memmap_t *memmap) {
    if (!efiMap || !memmap || !memmap->pDescriptors) return EFI_INVALID_PARAMETER;

    UINT8 *base = (UINT8 *) (UINTN) memmap->pDescriptors;
    UINT8 *end  = base + memmap->bufferSize;

    UINTN inCount        = efiMap->size / efiMap->descSize;
    UINTN outBytesNeeded = inCount * sizeof(bi_memdesc_t);

    if (base + outBytesNeeded > end) { return EFI_BUFFER_TOO_SMALL; }

    memmap->descriptorCount = inCount;

    for (UINTN i = 0; i < inCount; i++) {
        const EFI_MEMORY_DESCRIPTOR *efiDesc =
                (const EFI_MEMORY_DESCRIPTOR *) ((const UINT8 *) efiMap->descs + i * efiMap->descSize);
        bi_memdesc_t *biDesc = (bi_memdesc_t *) (base + i * sizeof(bi_memdesc_t));

        if ((UINT8 *) biDesc + sizeof(bi_memdesc_t) > end) { return EFI_BUFFER_TOO_SMALL; }

        biDesc->base      = efiDesc->PhysicalStart;
        biDesc->pageCount = efiDesc->NumberOfPages;
        biDesc->type      = memtype_efi_to_bi(efiDesc->Type);
        biDesc->flags     = 0;
    }

    return EFI_SUCCESS;
}
