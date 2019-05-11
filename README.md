<h1 align="center"> Manticore Operating System</h1>

<p align="center">
<a href="https://semaphoreci.com/manticore/manticore">
  <img src="https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/shields_badge.svg" alt="Build Status">
</a>
<a href="https://choosealicense.com/licenses/mit">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square" alt="MIT License">
</a>
<a href="https://choosealicense.com/licenses/apache-2.0">
  <img src="https://img.shields.io/badge/license-apache-blue.svg?style=flat-square" alt="Apache License">
</a>
</p>

<p align="center">
  A research operating system to explore <a href="http://penberg.org/parakernel-hotos19.pdf"><i>parakernels</i></a>.
</p>

## Table of Contents

* [Introduction](#introduction)
* [Features](#features)
* [Getting Started](#getting-started)
  * [Building from Sources](#building-from-sources)
  * [Supported Hardware](#supported-hardware)
  * [Code Structure](#code-structure)
* [Documentation](#documentation)
* [Publications](#publications)
* [Contributing](#contributing)
* [Authors](#authors)
* [License](#license)

## Introduction

Manticore is a research operating system, written in Rust, with the aim of exploring the [parakernel](http://penberg.org/parakernel-hotos19.pdf) OS architecture.

The OS is increasingly a bottleneck for server applications that want to take maximum advantage of the hardware.
Many traditional kernel interfaces (such as in POSIX) were designed when I/O was significantly slower than the CPU.
However, today I/O is getting faster, but single-threaded CPU performance has stagnated.
For example, a 40 GbE NIC can receive a cache-line sized packet faster than the CPU can access its last-level cache (LLC), which makes it tricky for an OS to keep up with packets arriving from the network.
Similarly, non-volatile memory (NVM) access speed is getting closer to DRAM speeds, which challenges OS abstractions for storage.

To address this OS bottleneck, server applications are increasingly adopting kernel-bypass techniques.
For example, the [Seastar framework](http://seastar.io/) is an OS implemented in userspace, which implements its own CPU and I/O scheduler, and bypasses the Linux kernel as much as it can.
Parakernel is an OS architecture that eliminates many OS abstractions (similar to _exokernels_) and partitions hardware resources (similar to _multikernels_) to facilitate high-performance server application with increased application-level parallelism and predictable tail latency.

## Features

 * Process scheduling (no kernel threads)
 * Hardware resource partitioning
 * Virtual memory (no demand paging)
 * Kernel-bypass by default
 * Non-blocking OS system calls
 * ELF executable support

## Getting Started

### Building from Sources

First, install the toolchain, which includes [`rustup`](https://rustup.rs/), Rust, and other dependencies:

```
./scripts/install-toolchain
```

Now that you have the toolchain installed, you can build Manticore with:

```
make
```

The build system generates a `kernel.iso` image, which you can launch under QEMU with:

```
$ ./scripts/run kernel.iso
```

For more information, see [Manticore Hacker's Guide](HACKING.md).

### Supported Hardware

 * Legacy-free PC with a 64-bit x86 processor
   * xAPIC2 interrupt controller
   * MSI-X interrupt delivery
   * PCIe 3.0 bus
 * VirtIO network device

### Code Structure

Manticore's code is structured into different directories as follows:

* [`arch`](./arch): machine architecture specific code
* [`drivers`](./drivers): device drivers
  * [`virtio`](./drivers/virtio): VirtIO device drivers
  * [`pci`](./drivers/pci): PCIe device drivers
* [`kernel`](./kernel): kernel services (e.g., process scheduling and system calls)
* [`lib`](./lib): support libraries
* [`mm`](./mm): memory management (e.g., virtual memory manager and kernel dynamic memory allocator)
* [`usr`](./usr): user space libraries and example applications
  
## Documentation

* [Manticore Hacker's Guide](HACKING.md)

## Publications

* Pekka Enberg, Ashwin Rao, and Sasu Tarkoma. 2019. [I/O Is Faster Than the CPU – Let’s Partition Resources and Eliminate (Most) OS Abstractions](http://penberg.org/parakernel-hotos19.pdf). HotOS '19

## Contributing

Bug reports and pull requests are welcome!

Please note that this project is released with a Contributor Code of Conduct. By participating in this project you agree to abide by its terms. See [Code of Coduct](code-of-conduct.md) for details.

## Authors

* [Pekka Enberg](https://penberg.org)

See also the list of [contributors](https://github.com/penberg/manticore/contributors) who contributed to this project.

## License

Licensed under either of these:

 * Apache License, Version 2.0, ([LICENSE-APACHE](LICENSE-APACHE) or https://www.apache.org/licenses/LICENSE-2.0)
 * MIT license ([LICENSE-MIT](LICENSE-MIT) or https://opensource.org/licenses/MIT)
   
Unless you explicitly state otherwise, any contribution you intentionally submit for inclusion in the work, as defined in the Apache-2.0 license, shall be dual-licensed as above, without any additional terms or conditions.
