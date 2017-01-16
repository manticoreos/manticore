CROSS_PREFIX ?= aarch64-linux-gnu-

CFLAGS += -mgeneral-regs-only

includes += -Iarch/aarch64/include

objs += arch/aarch64/cpu.o
objs += arch/aarch64/boot.o
objs += arch/aarch64/interrupts.o
objs += arch/aarch64/memory.o
objs += arch/aarch64/thread.o
objs += drivers/uart/pl011.o

include lib/libfdt/Makefile

all: kernel.bin

kernel.bin: kernel.elf
	$(CROSS_PREFIX)objcopy -O binary kernel.elf kernel.bin
