PROGRAMS = echod.linux echod.manti

CFLAGS = -Wall -O3 -g -static -fno-stack-protector

all: libs echod.linux echod.manti echod.iso

clean:
	rm -f $(PROGRAMS) echod.iso
	rm -rf build

libs:
	mkdir -p build && cd build && cmake ../.. && make
.PHONY: libs
	
echod.linux: echod.c
	gcc $(CFLAGS) echod.c -o echod.linux

echod.manti: libs echod.c
	gcc $(CFLAGS) -I../liblinux/include -nostdlib echod.c build/liblinux/liblinux.a build/libmanticore/libmanticore.a -o echod.manti

echod.iso: echod.manti ../../kernel.elf
	../../scripts/mkiso --kernel ../../kernel.elf --initrd echod.manti echod.iso
