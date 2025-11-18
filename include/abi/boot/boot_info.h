#ifndef _BOOT_INFO_H
#define _BOOT_INFO_H

#include <nyx/types.h>

#define BI_ABI   __attribute__((aligned(8)))
#define BI_MAGIC {0x4E, 0x42, 0x36, 0x34}
#define BI_MAGIC0 0x4E
#define BI_MAGIC1 0x42
#define BI_MAGIC2 0x36
#define BI_MAGIC3 0x34

typedef u64 bi_physaddress_t;
typedef u64 bi_virtaddress_t;

typedef enum {
    BI_STRUCTURE_TYPE_BOOT_INFO,
} bi_structtype;

typedef enum {
    BI_VERSION_INVALID = 0x0,
    BI_VERSION_1       = 0x1,
} bi_version;

typedef enum {
    BI_BOOTFLAGS_EFI_BOOT     = 1 << 0,
    BI_BOOTFLAGS_BIOS_BOOT    = 1 << 1,
    BI_BOOTFLAGS_BOOT_PARTIAL = 1 << 2,
} bi_flags;

typedef enum {
    BI_MEMTYPE_USABLE,
    BI_MEMTYPE_RECLAIMABLE,
    BI_MEMTYPE_RESERVED,
    BI_MEMTYPE_UNUSABLE,
    BI_MEMTYPE_ACPI_RECLAIMABLE,
    BI_MEMTYPE_ACPI_NVS,
    BI_MEMTYPE_MMIO,
    BI_MEMTYPE_MMIO_PORT_SPACE,
    BI_MEMTYPE_PERSISTENT_MEM,
    BI_MEMTYPE_EFI_RT_CODE,
    BI_MEMTYPE_EFI_RT_DATA,
    BI_MEMTYPE_UNKNOWN,
} bi_memtype;

typedef enum {
    BI_MEMFLAG_READ    = 1 << 0,
    BI_MEMFLAG_WRITE   = 1 << 1,
    BI_MEMFLAG_EXECUTE = 1 << 2,
} bi_memflags;

typedef struct bi_memdesc {
    bi_physaddress_t base;     /// base physical address of the region
    u64              pageCount;/// number of 4 KiB pages occupied by the region
    u32              type;     /// bi_memtype of the memory region
    u32              flags;    /// bi_memflags of the memory region
} BI_ABI bi_memdesc_t;

typedef struct bi_memmap {
    u32              version;        /// version of this structure
    u32              _pad0;          /// padding
    u64              bufferSize;     /// size of the pDescriptors buffer in bytes
    u64              descriptorSize; /// size of one descriptor in the descriptor array in bytes
    u64              descriptorCount;/// number of descriptors in the descriptor array
    bi_physaddress_t pDescriptors;   /// physical address of a bi_memdesc_t array
} BI_ABI bi_memmap_t;

typedef struct bi_kernelmapdesc {
    bi_physaddress_t paddr;    /// physical address the kernel section was loaded into
    bi_virtaddress_t vaddr;    /// virtual address the kernel section is mapped to
    u64              pageCount;/// number of 4 KiB pages the kernel section occupies
    u32              flags;    /// bi_memflags of the kernel section
    u32              _pad0;    /// padding
} BI_ABI bi_kernelmapdesc_t;

typedef struct bi_kernelmap {
    u32              version;        /// version of this structure
    u32              _pad0;          /// padding
    u64              descriptorSize; /// size of one descriptor in the descriptor array in bytes
    u64              descriptorCount;/// number of kernel section descriptors in the descriptor array
    bi_physaddress_t pDescriptors;   /// physical address of a bi_kernelmapdesc_t array
} BI_ABI bi_kernelmap_t;

typedef struct bi_head_struct {
    u32              structureType;
    u32              version;
    bi_physaddress_t pNext;
} BI_ABI bi_head;

typedef struct bi_bootinfo {
    bi_head          head;      // structure type must be BI_STRUCTURE_TYPE_BOOT_INFO
    u8               magic[4];  /// must be BI_MAGIC (0x4D444249ULL)
    u32              flags;     /// additional boot information flags
    u64              selfSize;  /// size of this structure
    bi_physaddress_t pMemMap;   /// physical address of bi_memmap_t
    bi_physaddress_t pKernelMap;/// physical address of bi_kernelmap_t
} BI_ABI bi_bootinfo_t;

#endif// !NYX_BOOT_INFO_H
