ARCH ?= $(shell uname -m)

include arch/$(ARCH)/Makefile
include lib/libc/Makefile

includes += -include include/kernel/kernel.h -Iinclude

objs += kernel/init.o
objs += kernel/panic.o
objs += kernel/printf.o
objs += kernel/thread.o
objs += lib/compiler-rt-stubs.o

CFLAGS += -O3 -g -Wall -ffreestanding $(includes)
ASFLAGS += -D__ASSEMBLY__ $(includes)

LIBKERNEL=target/$(ARCH)-unknown-none/release/libkernel.a

all: kernel.bin

DEPS=.deps
$(objs): | $(DEPS)
$(DEPS):
	mkdir -p $(DEPS)

kernel.bin: kernel.elf
	$(CROSS_PREFIX)objcopy -O binary kernel.elf kernel.bin

kernel.elf: $(objs) $(LIBKERNEL)
	$(CROSS_PREFIX)ld $(LDFLAGS) -Tarch/$(ARCH)/kernel.ld $^ -o $@ -Ltarget/$(ARCH)-unknown-none/release -lkernel

$(LIBKERNEL):
	RUST_TARGET_PATH=$(PWD) xargo build --release --target $(ARCH)-unknown-none

%.o: %.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -MD -c -o $@ $< -MF $(DEPS)/$(notdir $*).d

%.o: %.S
	$(CROSS_PREFIX)gcc $(ASFLAGS) -MD -c $< -o $@ -MF $(DEPS)/$(notdir $*).d

clean:
	rm -f kernel.elf $(objs)
	rm -rf target
	rm -rf $(DEPS)

.PHONY: all clean

-include $(DEPS)/*.d
