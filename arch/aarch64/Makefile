CROSS_PREFIX ?= aarch64-linux-gnu-

CFLAGS += -mgeneral-regs-only

includes += -Iarch/aarch64/include

objs += arch/aarch64/cpu.o
objs += arch/aarch64/boot.o
objs += arch/aarch64/memory.o
objs += arch/aarch64/mmu.o
objs += arch/aarch64/thread.o
objs += arch/aarch64/setup.o
objs += arch/aarch64/stubs.o
objs += drivers/uart/pl011.o

include lib/libfdt/Makefile

all: kernel.bin

kernel.bin: kernel.elf
	$(E) "  OBJCOPY " $@
	$(Q) $(CROSS_PREFIX)objcopy -O binary kernel.elf kernel.bin

archclean:
	$(Q) rm -f kernel.bin

.PHONY: archclean
