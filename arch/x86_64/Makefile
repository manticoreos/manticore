CFLAGS += -mno-sse -mno-red-zone
LDFLAGS += -n

includes += -Iarch/x86_64/include

objs += arch/x86_64/cpu.o
objs += arch/x86_64/boot.o
objs += arch/x86_64/entry.o
objs += arch/x86_64/exceptions.o
objs += arch/x86_64/memory.o
objs += arch/x86_64/mmu.o
objs += arch/x86_64/multiboot_header.o
objs += arch/x86_64/thread.o
objs += drivers/uart/8250.o

all: kernel.iso

kernel.iso: kernel.elf
	./scripts/build-iso
