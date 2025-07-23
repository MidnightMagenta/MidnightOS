# MidnightOS
## Warning
This code runs in R0 (supervisor mode). It has not been extensively tested on real hardware. It may inadevertantly cause irreversible damage to the computer. Building and running this software on real hardware shall be done only at your own risk.

### Build instructions
 - clone, build, and install the [signing tools](https://github.com/MidnightMagenta/MidnightSign.git)
 - build GCC with the following flags: `--target=x86_64-elf --disable-nls --enable-languages=c,c++ --without-headers --prefix=$(path_to_install_gcc)`
 - build binutils with the following flags: `--target=x86_64-elf --with-sysroot --disable-nls --disable-werror --prefix=$(path_to_install_binutils)`
 - from the root directory of this project run `make gen-keys`
 - from the root directory of this project run `make all`
 - from the root directory of this project run `make run` to run the software via QEMU

Rebuilding the project without re-creating the disk image may be done via `make partial`

This project does not support being burnt onto a bootable disk yet. Any attempt to run any build output of this project on real hardware shall be done at your own risk.
