ARCH ?= $(shell uname -m | sed -e "s/x86_64/x86/g")

include arch/$(ARCH)/Makefile

includes += -Iinclude

objs += kernel/init.o

CFLAGS += -O3 -Wall -ffreestanding $(includes)

all: kernel.elf

kernel.elf: $(objs)
	$(CROSS_PREFIX)ld $(LDFLAGS) -Tarch/$(ARCH)/kernel.ld $^ -o $@

%.o: %.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CROSS_PREFIX)gcc $(ASFLAGS) -c $< -o $@

clean:
	rm -f kernel.elf $(objs)
