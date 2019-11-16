ARCH ?= $(shell uname -m)

# Make the build silent by default
V =
ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q

include arch/$(ARCH)/Makefile
include lib/libc/Makefile

LD := ld.bfd

#
# The filename of the kernel image. This is the main output of the build
# system, which contains an ELF image of the operating system kernel.
#
KERNEL_IMAGE = kernel.elf

includes += -include include/kernel/kernel.h -Iinclude

objs += kernel/atomic-ring-buffer.o
objs += kernel/init.o
objs += kernel/initrd.o
objs += kernel/panic.o
objs += kernel/printf.o
objs += kernel/syscall.o
objs += kernel/thread.o
objs += kernel/user-copy.o
objs += mm/kmem.o

rust_src += drivers/pci/lib.rs
rust_src += drivers/virtio/lib.rs
rust_src += drivers/virtio/net.rs
rust_src += drivers/virtio/virtqueue.rs
rust_src += kernel/device.rs
rust_src += kernel/event.rs
rust_src += kernel/ioport.rs
rust_src += kernel/lib.rs
rust_src += kernel/memory.rs
rust_src += kernel/print.rs
rust_src += kernel/process.rs
rust_src += kernel/sched.rs
rust_src += kernel/vm.rs
rust_src += manticore.rs

ifdef TEST
CFLAGS += -DHAVE_TEST
tests += tests/tst-kmem.o
tests += tests/tst-page-alloc.o
tests += tests/tst-printf.o
endif

WARNINGS = -Wall -Wextra -Wno-unused-parameter
CFLAGS += -std=gnu11 -O3 -g $(WARNINGS) -ffreestanding $(includes) -fno-pie -fno-stack-protector
ASFLAGS += -D__ASSEMBLY__ $(includes)
LDFLAGS += --gc-sections

LIBMANTICORE=target/$(ARCH)-unknown-none/release/libmanticore.a

DEPS=.deps
$(objs): | $(DEPS)
$(DEPS):
	$(Q) mkdir -p $(DEPS)

all: $(KERNEL_IMAGE) usr/echod/echod.iso

$(KERNEL_IMAGE): arch/$(ARCH)/kernel.ld $(objs) $(LIBMANTICORE) $(tests)
	$(E) "  LD      " $@
	$(Q) $(CROSS_PREFIX)$(LD) $(LDFLAGS) -Tarch/$(ARCH)/kernel.ld $(objs) $(LIBMANTICORE) $(tests) -o $@ -Ltarget/$(ARCH)-unknown-none/release -lmanticore

$(LIBMANTICORE): $(rust_src)
	$(E) "  XARGO"
	$(Q) CC=$(CROSS_PREFIX)gcc RUST_TARGET_PATH=$(PWD) xargo build --release --target $(ARCH)-unknown-none

%.o: %.c
	$(E) "  CC      " $@
	$(Q) $(CROSS_PREFIX)gcc $(CFLAGS) -MD -c -o $@ $< -MF $(DEPS)/$(notdir $*).d

%.o: %.S
	$(E) "  AS      " $@
	$(Q) $(CROSS_PREFIX)gcc $(ASFLAGS) -MD -c $< -o $@ -MF $(DEPS)/$(notdir $*).d

%.ld: %.ld.S
	$(E) "  CPP     " $@
	$(Q) $(CROSS_PREFIX)cpp $(CFLAGS) -P $< $@

usr/echod/echod.iso: $(KERNEL_IMAGE)
	$(E) "  MAKE -C usr/echod"
	$(Q) make -C usr/echod

clean: archclean
	$(E) "  CLEAN"
	$(Q) rm -f $(KERNEL_IMAGE) $(objs) $(tests)
	$(Q) rm -f arch/$(ARCH)/kernel.ld
	$(Q) rm -rf target
	$(Q) rm -rf $(DEPS)

.PHONY: all clean

-include $(DEPS)/*.d
