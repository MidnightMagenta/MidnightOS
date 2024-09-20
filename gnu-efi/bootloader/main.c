#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable) {
	InitializeLib(imageHandle, systemTable);
	Print(L"Hello bootloader!\n\r");
	return EFI_SUCCESS;
}