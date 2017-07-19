# Kernel

[![Build Status](https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/badge.svg)](https://semaphoreci.com/kernel/kernel)


A prototype OS for network intensive, single-board multicore systems that aims to offer secure, fast, and energy-efficient communication and computation.

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
