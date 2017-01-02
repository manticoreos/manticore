#!/bin/bash

ARCH=$(readelf -h kernel.elf | grep "Machine:" | sed "s/.*Machine:\s*\(.*\)/\1/g")

case "$ARCH" in
    AArch64)
        qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel kernel.bin
        ;;
    Advanced\ Micro\ Devices\ X86-64)
        qemu-system-x86_64 -nographic -enable-kvm -cdrom kernel.iso
        ;;
    *)
        echo "error: Unsupported architecture: '$ARCH', exiting ..."
        ;;
esac
