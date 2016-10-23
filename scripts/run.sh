#!/bin/bash

ARCH=$(readelf -h kernel.elf | grep "Machine:" | sed "s/.*Machine:\s*\(.*\)/\1/g")

case "$ARCH" in
    AArch64)
        qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel kernel.elf
        ;;
    Intel\ 80386)
        qemu-system-i386 -nographic -enable-kvm -kernel kernel.elf
        ;;
    *)
        echo "error: Unsupported architecture: '$ARCH', exiting ..."
        ;;
esac
