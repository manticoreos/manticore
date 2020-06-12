<p align="center">
<img src="docs/images/manticore.png">
<p>

<p align="center">
<a href="https://circleci.com/gh/manticoreos/manticore">
  <img src="https://circleci.com/gh/manticoreos/manticore/tree/master.svg?style=svg" alt="Build Status">
</a>
<img src="https://img.shields.io/badge/license-MIT%2FApache--2.0-blue.svg" alt="MIT/Apache 2.0 License">
<a href="https://gitter.im/manticoreos/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge">
  <img src="https://badges.gitter.im/manticoreos/community.svg" alt="Gitter">
</a>
</p>

<p align="center">
  Manticore is a research operating system to explore <a href="http://penberg.org/parakernel-hotos19.pdf"><i>parakernels</i></a>.
</p>

[![asciicast](https://asciinema.org/a/WBQaRkKvKv35RDIWl0QXYNssw.svg)](https://asciinema.org/a/WBQaRkKvKv35RDIWl0QXYNssw)

## Table of Contents

* [Introduction](#introduction)
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

Manticore is a clean-slate research operating system, written in the [Rust programming language](https://www.rust-lang.org/), with the aim of exploring the [parakernel](http://penberg.org/parakernel-hotos19.pdf) OS architecture.

Please refer to the [project homepage](https://manticoreos.io/) for more information.

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

### Running Example Applications

Once `kernel.elf` is built, you can build an user space echo server with:

```
$ make -C usr/echod
```

and launch it under QEMU with:

```
$ ./scripts/run usr/echod/echod.iso
```

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
