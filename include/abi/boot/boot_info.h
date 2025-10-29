#ifndef MDOS_BOOT_INFO_H
#define MDOS_BOOT_INFO_H

#include <stdint.h>

#define BI_ASSERT_ABI_CONSISTENT(s) _Static_assert(_Alignof(s) == 8, #s " alignment is not 8 bytes.")

#define BI_MAGIC 0x4D444249UL

typedef uint64_t bi_physaddress_t;
typedef uint64_t bi_virtaddress_t;

typedef enum {
  BI_STRUCTURE_TYPE_BOOT_INFO,
} bi_structtype;

typedef enum {
  BI_VERSION_INVALID = 0x0,
  BI_VERSION_1 = 0x1,
} bi_version;

typedef enum {
  BI_BOOTFLAGS_EFI_BOOT = 1 << 0,
  BI_BOOTFLAGS_BIOS_BOOT = 1 << 1,
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
  BI_MEMFLAG_READ = 1 << 0,
  BI_MEMFLAG_WRITE = 1 << 1,
  BI_MEMFLAG_EXECUTE = 1 << 2,
} bi_memflags;

typedef struct bi_memdesc {
  bi_physaddress_t base;/// base physical address of the region
  uint64_t pageCount;   /// number of 4 KiB pages occupied by the region
  uint32_t type;        /// bi_memtype of the memory region
  uint32_t flags;       /// bi_memflags of the memory region
} bi_memdesc_t;
BI_ASSERT_ABI_CONSISTENT(bi_memdesc_t);

typedef struct bi_memmap {
  uint32_t version;             /// version of this structure
  uint32_t _pad0;               /// padding
  uint64_t bufferSize;          /// size of the pDescriptors buffer in bytes
  uint64_t descriptorSize;      /// size of one descriptor in the descriptor array in bytes
  uint64_t descriptorCount;     /// number of descriptors in the descriptor array
  bi_physaddress_t pDescriptors;/// physical address of a bi_memdesc_t array
} bi_memmap_t;
BI_ASSERT_ABI_CONSISTENT(bi_memmap_t);

typedef struct bi_kernelmapdesc {
  bi_physaddress_t paddr;/// physical address the kernel section was loaded into
  bi_virtaddress_t vaddr;/// virtual address the kernel section is mapped to
  uint64_t pageCount;    /// number of 4 KiB pages the kernel section occupies
  uint32_t flags;        /// bi_memflags of the kernel section
  uint32_t _pad0;        ///padding
} bi_kernelmapdesc_t;
BI_ASSERT_ABI_CONSISTENT(bi_kernelmapdesc_t);

typedef struct bi_kernelmap {
  uint32_t version;             /// version of this structure
  uint32_t _pad0;               /// padding
  uint64_t descriptorSize;      /// size of one descriptor in the descriptor array in bytes
  uint64_t descriptorCount;     /// number of kernel section descriptors in the descriptor array
  bi_physaddress_t pDescriptors;/// physical address of a bi_kernelmapdesc_t array
} bi_kernelmap_t;
BI_ASSERT_ABI_CONSISTENT(bi_kernelmap_t);

typedef struct bi_bootinfo {
  uint32_t magic;             /// must be BI_MAGIC (0x4D444249ULL)
  uint32_t structureType;     /// must be BI_STRUCTURE_TYPE_BOOT_INFO
  uint32_t version;           /// version of this structure. Does not apply to following strucures
  uint32_t flags;             /// additional boot information flags
  uint64_t selfSize;          /// size of this structure
  bi_physaddress_t pMemMap;   /// physical address of bi_memmap_t
  bi_physaddress_t pKernelMap;/// physical address of bi_kernelmap_t
  bi_physaddress_t pNext;     /// physical address of the next boot info structure
} bi_bootinfo_t;
BI_ASSERT_ABI_CONSISTENT(bi_bootinfo_t);

#endif// !MDOS_BOOT_INFO_H
