#include "../include/bootinfo.h"
#include "../include/cfgparse.h"
#include "../include/debug.h"
#include "../include/elf.h"
#include "../include/fs.h"
#include "../include/memory.h"
#include "../include/types.h"
#include "../include/utils.h"
#include <efi.h>
#include <efilib.h>

#ifndef HAVE_USE_MS_ABI
#error "Compiler must support using MS ABI"
#endif

#define DIRECT_MAP_BASE      0xFFFF800000000000ULL
#define BI_MEMMAP_SIZE       8192
#define EXIT_BS_MAX_ATTEMPTS 5

EFI_STATUS load_boot_bin(EFI_HANDLE imageHandle, configinfo_t *bootConfig, elf_loadinfo_t *elfInfo) {
    EFI_STATUS res = EFI_SUCCESS;

    EFI_HANDLE                       bootPart   = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *bootPartFs = NULL;
    EFI_FILE                        *bootRoot   = NULL;
    EFI_FILE                        *bootBinary = NULL;

    res = parse_config(imageHandle, bootConfig);
    if (EFI_ERROR(res)) { return res; }

    res = find_filesystem_for_guid(&bootConfig->bootPartUUID, &bootPart, &bootPartFs);
    if (EFI_ERROR(res)) { return res; }

    res = bootPartFs->OpenVolume(bootPartFs, &bootRoot);
    if (EFI_ERROR(res)) { return res; }

    res = bootRoot->Open(bootRoot, &bootBinary, bootConfig->bootBinPath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(res)) { goto exit_free_root; }

    res = elf_load_file(bootBinary, elfInfo, ELF_LOAD_DISCONTIG);
    if (EFI_ERROR(res)) { goto exit_free_bootFile; }

exit_free_bootFile:
    bootBinary->Close(bootBinary);
exit_free_root:
    bootRoot->Close(bootRoot);
    return res;
}

EFI_STATUS map_memory(uint64_t *pml4, efi_memmap_t *const memMap, elf_loadinfo_t *const loadInfo) {
    for (size_t i = 0; i < memMap->size / memMap->descSize; ++i) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) ((char *) memMap->descs + i * memMap->descSize);
        if (desc->Type == EfiUnusableMemory || desc->Type == EfiReservedMemoryType || desc->Type == EfiMemoryMappedIO ||
            desc->Type == EfiMemoryMappedIOPortSpace || desc->Type == EfiPalCode || desc->Type == EfiPersistentMemory) {
            continue;
        }

        EFI_TRY_RET(mem_map_pages(pml4, desc->PhysicalStart, desc->PhysicalStart, desc->NumberOfPages));
        EFI_TRY_RET(
                mem_map_pages(pml4, desc->PhysicalStart + DIRECT_MAP_BASE, desc->PhysicalStart, desc->NumberOfPages));
    }

    for (size_t i = 0; i < loadInfo->sectionCount; ++i) {
        elf_sectioninfo_t *section =
                (elf_sectioninfo_t *) ((char *) loadInfo->sections + i * sizeof(elf_sectioninfo_t));

        EFI_TRY_RET(mem_map_pages(pml4, section->reqVirt, section->phys, section->pageCount));
    }

    return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
    InitializeLib(imageHandle, systemTable);
    EFI_STATUS res = EFI_SUCCESS;

    configinfo_t         bootConfig       = {0};
    elf_loadinfo_t       elfInfo          = {0};
    efi_memmap_t         memMap           = {0};
    size_t               memMapBufferSize = 0;
    EFI_PHYSICAL_ADDRESS pml4Addr         = 0;
    uint64_t            *pml4             = NULL;
    bi_bootinfo_t       *bootInfo         = NULL;

    // load the boot binary
    res = load_boot_bin(imageHandle, &bootConfig, &elfInfo);
    if (EFI_ERROR(res)) { goto err_free_none; }

    // map the boot binary to it's expected vaddr
    res = efi_get_memmap(&memMap, &memMapBufferSize);
    if (EFI_ERROR(res)) { goto err_free_loadedBin; }

    res = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4Addr);
    if (EFI_ERROR(res)) { goto err_free_map; }

    pml4 = (uint64_t *) pml4Addr;
    ZeroMem(pml4, 0x1000);

    res = map_memory(pml4, &memMap, &elfInfo);
    if (EFI_ERROR(res)) { goto err_free_pageTables; }

    // create the boot info structure
    bi_bootinfo_createinfo bootInfoCreateInfo = {0};
    bootInfoCreateInfo.elfSections            = &elfInfo;
    res                                       = bi_build_bootinfo(&bootInfoCreateInfo, &bootInfo);
    if (EFI_ERROR(res)) { goto err_free_pageTables; }

    // exit boot services
    DBG_MSG("Exiting boot services\n\r");
    for (int attempt = 0; attempt < EXIT_BS_MAX_ATTEMPTS; ++attempt) {
        res = efi_get_memmap_norealloc(memMapBufferSize, &memMap);
        if (EFI_ERROR(res)) { continue; }
        res = gBS->ExitBootServices(imageHandle, memMap.key);
        if (!EFI_ERROR(res)) { break; }
    }
    if (EFI_ERROR(res)) {
        // try reallocating memory map
        efi_free_memmap(&memMap);
        res = efi_get_memmap(&memMap, &memMapBufferSize);
        if (EFI_ERROR(res)) { goto err_halt; }
        res = gBS->ExitBootServices(imageHandle, memMap.key);
        if (EFI_ERROR(res)) { goto err_halt; }
    }

    res = bi_update_memmap(&memMap, (bi_memmap_t *) bootInfo->pMemMap);
    if (EFI_ERROR(res)) { goto err_halt; }

    // pass control to the loaded binary
    __asm__ volatile("cli\n"
                     "mov %0, %%cr3\n"
                     "mov %1, %%rdi\n"
                     "jmp *%2\n"
                     :
                     : "r"(pml4Addr), "r"(bootInfo), "r"(elfInfo.entry)
                     : "rdi", "memory");

err_halt:
    while (1) { __asm__ volatile("hlt"); }

    // abnormal exit path - only valid before exiting boot services
err_free_pageTables:
    mem_free_tables(pml4);
    gBS->FreePages((EFI_PHYSICAL_ADDRESS) (UINTN) pml4, 1);
err_free_map:
    efi_free_memmap(&memMap);
err_free_loadedBin:
    elf_free_sections(&elfInfo);
    elf_free_loadinfo(&elfInfo);
    free_config(&bootConfig);
err_free_none:
    return res;
}