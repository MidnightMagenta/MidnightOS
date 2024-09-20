SRCDIR = .

VPATH = $(SRCDIR)

include $(SRCDIR)/../Make.defaults

TOPDIR = $(SRCDIR)/..

CDIR=$(TOPDIR)/..
LINUX_HEADERS	= /usr/src/sys/build
CPPFLAGS	+= -D__KERNEL__ -I$(LINUX_HEADERS)/include
CRTOBJS		= ../gnuefi/crt0-efi-$(ARCH).o

LDSCRIPT	= $(TOPDIR)/gnuefi/elf_$(ARCH)_efi.lds
ifneq (,$(findstring FreeBSD,$(OS)))
LDSCRIPT	= $(TOPDIR)/gnuefi/elf_$(ARCH)_fbsd_efi.lds
endif

LDFLAGS		+= -shared -Bsymbolic -L../lib -L../gnuefi $(CRTOBJS)

LOADLIBES	+= -lefi -lgnuefi
LOADLIBES	+= $(LIBGCC)
LOADLIBES	+= -T $(LDSCRIPT)

TARGET_APPS = main.efi
TARGET_BSDRIVERS =
TARGET_RTDRIVERS =

ifneq ($(HAVE_EFI_OBJCOPY),)

FORMAT		:= --target efi-app-$(ARCH)
$(TARGET_BSDRIVERS): FORMAT=--target efi-bsdrv-$(ARCH)
$(TARGET_RTDRIVERS): FORMAT=--target efi-rtdrv-$(ARCH)

else

SUBSYSTEM	:= 0xa
$(TARGET_BSDRIVERS): SUBSYSTEM = 0xb
$(TARGET_RTDRIVERS): SUBSYSTEM = 0xc

FORMAT		:= -O binary
LDFLAGS		+= --defsym=EFI_SUBSYSTEM=$(SUBSYSTEM)

endif

TARGETS = $(TARGET_APPS) $(TARGET_BSDRIVERS) $(TARGET_RTDRIVERS)

CFLAGS += -Wno-error=unused-parameter -Wno-error=unused-variable

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *~ *.o *.so

.PHONY: install

include $(SRCDIR)/../Make.rules