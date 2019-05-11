PROGRAMS = echod.linux echod.manti

all: libs echod.linux echod.manti echod.iso

clean:
	rm -f $(PROGRAMS) echod.iso
	rm -rf build

libs:
	mkdir -p build && cd build && cmake ../.. && make
	
echod.linux: echod.c
	gcc -Wall -O3 -g echod.c -o echod.linux

echod.manti: echod.c
	gcc -Wall -O3 -g -I../liblinux/include -ffreestanding -static --entry=main -Wl,--gc-sections -nostdlib echod.c build/liblinux/liblinux.a build/libmanticore/libmanticore.a -o echod.manti

echod.iso: echod.manti
	../../scripts/mkiso --kernel ../../kernel.elf --initrd echod.manti echod.iso
