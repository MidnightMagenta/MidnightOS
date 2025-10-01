#!/usr/bin/env bash
set -eu

IMAGE=${1:?Usage: $0 disk.img build_dir}
BUILD_DIR=${2:?}
FILES_DIR=${3:?}

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
sudo mkdir -p "$MNT/MdOS/bin"
sudo rsync -a --no-owner --no-group "$BUILD_DIR/bootloader/BOOTX64.EFI" "$MNT/EFI/BOOT";
sudo rsync -a --no-owner --no-group "$BUILD_DIR/bootloader/MDOSBOOT.EFI" "$MNT/BOOT";
sudo rsync -a --no-owner --no-group "$BUILD_DIR/mdoskrnl.elf" "$MNT/MdOS/bin";
sync

echo "[4/4] cleaning up"
