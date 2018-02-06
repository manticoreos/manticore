# Manticore [![Build Status](https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/badge.svg)](https://semaphoreci.com/manticore/manticore)

Manticore is an experimental OS for edge computing.

<p align="center">
  <img src="assets/manticore.png">
</p>

## Introduction

Manticore is an OS that has the following, sometimes conflicting, goals:

* **Security**: The OS provides a foundation for secure computing. It's critical that the OS leverages hardware protection capabilities, provides minimal and safe system call interface to minimize attack surface, and is itself implemented in high-level language to minimize exploitable bugs.
* **Low latency**: One of the main motivations of edge computing is to provide lower latency than an equivalent cloud-based service would have. It's important that the OS itself has low overheads and provides interfaces that support low-latency applications.
* **Energy efficiency**: Communications technology is forecast to consume around 20% of global electricity by 2030, or as much as 50% in the worst case (Andrae and Edler, 2015)! It's therefore important that the OS provides interfaces that enable energy efficient computing.

## Status

The kernel boots on QEMU/KVM platform on x86-64. The port to AArch64 boots under QEMU emulation but lacks most of the functionality of the x86-64 port.

### Features and Roadmap

 * [ ] Multikernel architecture (Baumann _et al_., 2009)
 * [ ] Kernel-bypass networking
 * [x] Memory management:
   - [x] Virtual memory
   - [x] Physical memory allocator
   - [x] Slab allocator
 * [x] ELF object file support

### Hardware Requirements

 * [x] Legacy-free PC with x86-64 processor -or-
 * [ ] Raspberry Pi 3 with AArch64 processor

### Supported Devices

 * [ ] Virtio network device 
 * [x] PCI bus
 * [x] 16550 UART
 * [x] PrimeCell® UART (PL011)

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

## Acknowledgments

* The [Manticore logo](https://commons.wikimedia.org/wiki/File:DnD_Manticore.png) is designed by Mariana Ruiz Villarreal and distributed under [Creative Commons CC0 1.0 Universal Public Domain Dedication](https://creativecommons.org/publicdomain/zero/1.0/deed.en).

## References

Anders S. G. Andrae and Tomas Edler. 2015. On Global Electricity Usage of Communication Technology: Trends to 2030. In _Challenges_, 6(1):117–157, 2015. DOI: http://dx.doi.org/10.3390/challe6010117

Andrew Baumann, Paul Barham, Pierre-Evariste Dagand, Tim Harris, Rebecca Isaacs, Simon Peter, Timothy Roscoe, Adrian Schüpbach, and Akhilesh Singhania. 2009. The multikernel: a new OS architecture for scalable multicore systems. In _Proceedings of the ACM SIGOPS 22nd symposium on Operating systems principles_ (SOSP '09). ACM, New York, NY, USA, 29-44. DOI: http://dx.doi.org/10.1145/1629575.1629579
