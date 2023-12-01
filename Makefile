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
objs += mm/mmu.o

#
# The filename of the kernel static library. This static library is the output
# of the `cargo` build of kernel Rust source code. The library is linked to
# the built C source code to produce the final kernel image.
#
KERNEL_LIB = target/$(ARCH)-unknown-none/release/libmanticore.a

#
# The source files for the kernel static library. Although `cargo` manages the
# build of the static library, we need to define Rust source files for `make`
# to calculate dependencies correctly to execute `cargo` when sources change.
#
KERNEL_LIB_SRC += drivers/pci/lib.rs
KERNEL_LIB_SRC += drivers/virtio/lib.rs
KERNEL_LIB_SRC += drivers/virtio/net.rs
KERNEL_LIB_SRC += drivers/virtio/virtqueue.rs
KERNEL_LIB_SRC += kernel/atomic_ring_buffer.rs
KERNEL_LIB_SRC += kernel/device.rs
KERNEL_LIB_SRC += kernel/errno.rs
KERNEL_LIB_SRC += kernel/event.rs
KERNEL_LIB_SRC += kernel/ioport.rs
KERNEL_LIB_SRC += kernel/ioqueue.rs
KERNEL_LIB_SRC += kernel/lib.rs
KERNEL_LIB_SRC += kernel/memory.rs
KERNEL_LIB_SRC += kernel/mmu.rs
KERNEL_LIB_SRC += kernel/print.rs
KERNEL_LIB_SRC += kernel/process.rs
KERNEL_LIB_SRC += kernel/sched.rs
KERNEL_LIB_SRC += kernel/vm.rs
KERNEL_LIB_SRC += manticore.rs

#
# The source code to manual pages.
#
MAN_PAGES += man/exit.txt
MAN_PAGES += man/get_config.txt
MAN_PAGES += man/vmspace_alloc.txt
MAN_PAGES += man/wait.txt

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

DEPS=.deps
$(objs): | $(DEPS)
$(DEPS):
	$(Q) mkdir -p $(DEPS)

all: $(KERNEL_IMAGE) usr/echod/echod.iso

$(KERNEL_IMAGE): arch/$(ARCH)/kernel.ld $(objs) $(KERNEL_LIB) $(tests)
	$(E) "  LD      " $@
	$(Q) $(CROSS_PREFIX)$(LD) $(LDFLAGS) -Tarch/$(ARCH)/kernel.ld $(objs) $(KERNEL_LIB) $(tests) -o $@ -Ltarget/$(ARCH)-unknown-none/release -lmanticore

$(KERNEL_LIB): $(KERNEL_LIB_SRC)
	$(E) "  XARGO"
	$(Q) CC=$(CROSS_PREFIX)gcc RUST_TARGET_PATH=$(PWD) cargo build --release --target $(ARCH)-unknown-none

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
	$(E) "  MAKE    " $@
	$(Q) make -s -C usr/echod
.PHONY: usr/echod/echod.iso

man: $(MAN_PAGES)
	$(E) "  ASCIIDOCTOR"
	$(Q) asciidoctor -b manpage $?
.PHONY: man

clean: archclean
	$(E) "  CLEAN"
	$(Q) rm -f $(KERNEL_IMAGE) $(objs) $(tests)
	$(Q) rm -f arch/$(ARCH)/kernel.ld
	$(Q) rm -rf target
	$(Q) rm -rf $(DEPS)
	$(Q) make --silent -C usr/echod clean

.PHONY: all clean

-include $(DEPS)/*.d
