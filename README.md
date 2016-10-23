# Kernel

## Features

* Runs on [arm64](http://www.arm.com/products/processors/armv8-architecture.php) and [x86](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html) architectures

## Getting Started

Install the arm64 cross compiling toolchain:

```console
$ ./scripts/install-aarch64-toolchain
```

Build the arm64 kernel image:

```console
$ make ARCH=arm64
```

Alternatively, you can build the x86 kernel image:

```console
$ make ARCH=x86
```

Run the kernel image under QEMU:

```console
$ ./scripts/run.sh
```

To build a bootable ISO image `kernel.iso`:

```console
$ ./scripts/build-iso
```
