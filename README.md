# Kernel

## Features

* Runs on [aarch64](http://www.arm.com/products/processors/armv8-architecture.php) and [x86_64](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html) architectures

## Getting Started

First, install Rust toolchain via [`rustup`](https://rustup.rs/).

Then, install a nightly build of Rust:

```console
$ rustup install nightly
$ rustup default nightly
$ rustup component add rust-src
```

Finally, install `xargo`:

```
$ cargo install xargo
```

To install the aarch64 cross compiling toolchain, run:

```console
$ ./scripts/install-aarch64-toolchain
```

Build the aarch64 kernel image:

```console
$ make ARCH=aarch64
```

Alternatively, you can build the x86_64 kernel image:

```console
$ make ARCH=x86_64
```

Run the kernel image under QEMU:

```console
$ ./scripts/run.sh
```
