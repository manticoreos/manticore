CFLAGS += -mno-sse -mno-red-zone -mcmodel=kernel
LDFLAGS += -n

includes += -Iarch/x86_64/include

objs += arch/x86_64/cpu.o
objs += arch/x86_64/boot.o
objs += arch/x86_64/entry.o
objs += arch/x86_64/exceptions.o
objs += arch/x86_64/i8259.o
objs += arch/x86_64/ioport.o
objs += arch/x86_64/memory.o
objs += arch/x86_64/mmu.o
objs += arch/x86_64/multiboot_header.o
objs += arch/x86_64/setup.o
objs += arch/x86_64/syscall.o
objs += arch/x86_64/thread.o
objs += arch/x86_64/task.o
objs += drivers/uart/8250.o

all: kernel.iso

kernel.iso: kernel.elf
	./scripts/build-iso
