#include <efi.h>
#include <efilib.h>

#define KERNEL_START_VADDR 0xFFFF800000000000

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	InitializeLib(imageHandle, systemTable);
	Print(L"Bootloader started\n\r");
	ST = systemTable;
	EFI_STATUS status;

	status = EFI_SUCCESS;
	return status;
}