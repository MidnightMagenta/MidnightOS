OSNAME = MidnightOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
LD = ld
CC = gcc

CFLAGS = -Iinclude -O0 -g -Wall -Wextra -ffreestanding -fshort-wchar
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

SRCDIR := src
OBJDIR := buil/objs
BUILDDIR = build/bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.cpp)          
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard $(SRCDIR)/*)

kernel: $(OBJS) link

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@

link:
	@ echo !==== LINKING
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(OBJS)

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(SRCDIR)
	@mkdir $(OBJDIR)

image:
	dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=512 count=93750
	mformat -F -i $(BUILDDIR)/$(OSNAME).img
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/efi
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/efi/boot
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/boot
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BOOTEFI) ::/efi/boot
	mren -i $(BUILDDIR)/$(OSNAME).img ::/efi/boot/main.efi ::/efi/boot/bootx64.efi
	mcopy -i $(BUILDDIR)/$(OSNAME).img startup.nsh ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/kernel.elf ::/boot

run:
	qemu-system-x86_64 \
	-drive file=$(BUILDDIR)/$(OSNAME).img,format=raw \
	-enable-kvm \
	-cpu host \
	-m 4G \
	-drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on \
	-drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" \
	-net none \

clean:
	@echo "!==== CLEANING BUILD FILES"
	rm -rf $(BUILDDIR)/* $(OBJDIR)/*