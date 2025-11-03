# MidnightOS

> [!CAUTION]
> This code runs in supervisor mode, and has access to all system resources. It has not been extensively tested on real hardware. It may inadvertently cause irreversible damage to the computer. Building and running this software on real hardware shall be done only at your own risk.

### Build instructions

> [!NOTE]
> These instructions are temporary, and may not work with future versions of the software

 - clone, build, and install the [additional tools](https://github.com/MidnightMagenta/MdOS-tools)
 - build GCC with the following flags: `--target=x86_64-elf --disable-nls --enable-languages=c --without-headers --prefix=$(path_to_install_gcc)`
 - build binutils with the following flags: `--target=x86_64-elf --with-sysroot --disable-nls --disable-werror --prefix=$(path_to_install_binutils)`
 - from the root directory of this project run `make`
 - from the root directory of this project run `make bootloader`
 - from the root directory of this project run `make image` to create a bootable image
 - from the root directory of this project run `make run` to run the OS in QEMU

Rebuilding the project without re-creating the disk image may be done via `make partial`

This project does not support being burnt onto a bootable disk yet. Any attempt to run any build output of this project on real hardware shall be done at your own risk.
