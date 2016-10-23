ARCH ?= $(shell uname -m | sed -e "s/x86_64/x86/g")

include arch/$(ARCH)/Makefile

includes += -Iinclude

objs += kernel/init.o

CFLAGS += -O3 -Wall -ffreestanding $(includes)

all: kernel.elf

DEPS=.deps
$(objs): | $(DEPS)
$(DEPS):
	mkdir -p $(DEPS)

kernel.elf: $(objs)
	$(CROSS_PREFIX)ld $(LDFLAGS) -Tarch/$(ARCH)/kernel.ld $^ -o $@

%.o: %.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -MD -c -o $@ $< -MF $(DEPS)/$(notdir $*).d

%.o: %.S
	$(CROSS_PREFIX)gcc $(ASFLAGS) -MD -c $< -o $@ -MF $(DEPS)/$(notdir $*).d

clean:
	rm -f kernel.elf $(objs)
	rm -rf $(DEPS)

.PHONY: all clean

-include $(DEPS)/*.d
