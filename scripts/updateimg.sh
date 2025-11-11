#!/usr/bin/env bash
set -euo pipefail

IMAGE=${1:?Usage: $0 disk.img build_dir}
BUILD_DIR=${2:?Usage: $0 disk.img build_dir}
FILES_DIR=${3:?Usage: $0 disk.img build_dir}

LOOPDEV=$(sudo losetup -Pf --show "$IMAGE")
PART=${LOOPDEV}p1
MNT=$(mktemp -d)

cleanup(){
	set +e
	sudo umount "$MNT" 2>/dev/null || true
	rmdir "$MNT" 2>/dev/null || true
	sudo losetup -d "$LOOPDEV" 2>/dev/null || true
}
trap cleanup EXIT

if [ ! -b "$PART" ]; then
    echo "Error: expected partition device $PART not found."
    exit 1
fi

echo "Loop device: $LOOPDEV"

echo "[1/4] formating $PART as FAT32"
sudo mkfs.vfat -F32 "$PART"

echo "[2/4] mounting $PART on $MNT"
sudo mount "$PART" "$MNT"

echo "[3/4] copying files"
sudo rsync -a --no-owner --no-group "$FILES_DIR/" "$MNT/";
sudo mkdir -p "$MNT/EFI/BOOT"
sudo mkdir -p "$MNT/BOOT"
sudo rsync -a --no-owner --no-group "$BUILD_DIR/bootloader/BOOTX64.EFI" "$MNT/EFI/BOOT";
sudo rsync -a --no-owner --no-group "$BUILD_DIR/bootloader/NYXBOOT.EFI" "$MNT/BOOT";
sudo rsync -a --no-owner --no-group "$BUILD_DIR/nyxos.elf" "$MNT/";
sync

echo "[4/4] cleaning up"
