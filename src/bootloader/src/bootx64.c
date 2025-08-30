#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	EFI_STATUS status;
	InitializeLib(imageHandle, systemTable);

	Print(L"Hello bootloader\n\r");

	return EFI_SUCCESS;
}