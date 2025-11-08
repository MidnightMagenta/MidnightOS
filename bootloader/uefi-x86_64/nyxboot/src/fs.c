#include "../include/fs.h"
#include "../include/debug.h"

static BOOLEAN match_node_guid(EFI_DEVICE_PATH_PROTOCOL *dp, EFI_GUID *guid) {
    for (EFI_DEVICE_PATH_PROTOCOL *node = dp; !IsDevicePathEnd(node); node = NextDevicePathNode(node)) {
        if (DevicePathType(node) == MEDIA_DEVICE_PATH && DevicePathSubType(node) == MEDIA_HARDDRIVE_DP) {
            HARDDRIVE_DEVICE_PATH *hdp = (HARDDRIVE_DEVICE_PATH *) node;
            if (hdp->SignatureType != SIGNATURE_TYPE_GUID) { continue; }

            EFI_GUID *sig = (EFI_GUID *) hdp->Signature;
            if (CompareGuid(sig, guid) == 0) { return TRUE; }
        }
    }

    return FALSE;
}

EFI_STATUS find_filesystem_for_guid(IN EFI_GUID                          *guid,
                                    OUT EFI_HANDLE                       *handle,
                                    OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **filesystem) {
    if (guid == NULL) { return EFI_INVALID_PARAMETER; }
    if (CompareGuid(guid, &NullGuid) == 0) { return EFI_INVALID_PARAMETER; }
    if (handle == NULL) { return EFI_INVALID_PARAMETER; }
    if (filesystem == NULL) { return EFI_INVALID_PARAMETER; }

    EFI_HANDLE *handles     = NULL;
    UINTN       handleCount = 0;
    EFI_STATUS  res;

    res = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handleCount, &handles);
    if (EFI_ERROR(res)) { return res; }

    for (UINTN i = 0; i < handleCount; i++) {
        EFI_DEVICE_PATH_PROTOCOL *dp = NULL;
        res                          = gBS->HandleProtocol(handles[i], &gEfiDevicePathProtocolGuid, (void **) &dp);
        if (EFI_ERROR(res) || dp == NULL) { continue; }

        if (match_node_guid(dp, guid)) {
            EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = NULL;
            res = gBS->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (void **) &sfs);

            if (EFI_ERROR(res) || sfs == NULL) { continue; }

            *handle     = handles[i];
            *filesystem = sfs;
            gBS->FreePool(handles);
            return EFI_SUCCESS;
        }
    }

    gBS->FreePool(handles);
    return EFI_NOT_FOUND;
}

EFI_STATUS find_filesystem_with_file(IN CHAR16                            *path,
                                     OUT EFI_HANDLE                       *handle,
                                     OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **filesystem) {
    if (path == NULL || handle == NULL || filesystem == NULL) { return EFI_INVALID_PARAMETER; }
    EFI_HANDLE *handles;
    UINTN       handleCount;
    EFI_STATUS  res;

    res = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handleCount, &handles);
    if (EFI_ERROR(res)) { return res; }
    if (handleCount == 0) {
        gBS->FreePool(handles);
        return EFI_NOT_FOUND;
    }

    for (UINTN i = 0; i < handleCount; i++) {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = NULL;
        res = gBS->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (void **) &fs);

        if (EFI_ERROR(res) || fs == NULL) { continue; }

        EFI_FILE *root = NULL;
        res            = fs->OpenVolume(fs, &root);
        if (EFI_ERROR(res) || root == NULL) { continue; }

        EFI_FILE *file = NULL;
        res            = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
        if (!EFI_ERROR(res) && file != NULL) {
            file->Close(file);
            root->Close(root);
            *handle     = handles[i];
            *filesystem = fs;
            gBS->FreePool(handles);
            return EFI_SUCCESS;
        }

        if (file != NULL) { file->Close(file); }
        root->Close(root);
    }

    gBS->FreePool(handles);
    return EFI_NOT_FOUND;
}

EFI_STATUS open_file(IN OPTIONAL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem,
                     IN OPTIONAL EFI_FILE                        *root,
                     IN CHAR16                                   *path,
                     IN UINT64                                    openMode,
                     IN OPTIONAL UINT64                           attributes,
                     OUT EFI_FILE                               **file) {
    if (root == NULL) {
        if (filesystem == NULL) { return EFI_INVALID_PARAMETER; }
        filesystem->OpenVolume(filesystem, &root);
    }

    return root->Open(root, file, path, openMode, attributes);
}

EFI_STATUS get_file_size(IN EFI_FILE *file, OUT UINTN *fileSize) {
    if (file == NULL || fileSize == NULL) { return EFI_INVALID_PARAMETER; }
    EFI_STATUS res;

    UINTN          fileInfoBufferSize = SIZE_OF_EFI_FILE_INFO + 200;
    EFI_FILE_INFO *fileInfo           = NULL;
    res                               = gBS->AllocatePool(EfiLoaderData, fileInfoBufferSize, (void **) &fileInfo);
    if (EFI_ERROR(res)) {
        DBG_MSG("[%a %d] Failed to get file info with: %lx\n\r", __func__, __LINE__, res);
        file->Close(file);
        return res;
    }

    res = file->GetInfo(file, &gEfiFileInfoGuid, &fileInfoBufferSize, fileInfo);
    if (EFI_ERROR(res)) {
        DBG_MSG("[%a %d] Failed to get file info with: %lx\n\r", __func__, __LINE__, res);
        gBS->FreePool(fileInfo);
        file->Close(file);
        return res;
    }

    *fileSize = fileInfo->FileSize;
    gBS->FreePool(fileInfo);
    return EFI_SUCCESS;
}
