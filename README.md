# Manticore [![Build Status](https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/badge.svg)](https://semaphoreci.com/manticore/manticore)

Manticore is an experimental OS for network-intensive servers, which require low-latency, high-throughput, and energy efficiency.

## Features

 * Memory management
   * Virtual memory
   * Physical memory allocator
   * Dynamic kernel memory allocator
 * ELF executable support

## Hardware Support

 * [x] Legacy-free PC with a 64-bit x86 processor:
   * [x] PCIe 3.0 bus
   * [x] xAPIC2 interrupt controller
   * [ ] MSI-X interrupt delivery
 * [ ] Raspberry Pi 3 with AArch64 processor:
   * [x] PrimeCell® UART (PL011)
 * [ ] Paravirtualized I/O drivers
   * [ ] VIRTIO network device

## Getting Started

First, install Rust toolchain installer, [`rustup`](https://rustup.rs/).

Then, install the toolchain:

```
./scripts/install-toolchain
```

You can now build Manticore with:

```
make
```

The command builds a bootable `kernel.iso` image.

To launch the image under a VM, run the following command:

```
$ ./scripts/run kernel.iso
```

### Cross Compiling to AArch64

You can install the AArch64 cross compiling toolchain with:

```
./scripts/install-aarch64-toolchain
```

To build an AArch64 kernel image, use:

```
make ARCH=aarch64
```

To run the AArch64 kernel image under QEMU:

```
./scripts/run-aarch64
```
