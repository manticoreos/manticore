# Manticore

[![Build Status](https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/badge.svg)](https://semaphoreci.com/manticore/manticore)

Manticore is an OS for network intensive, single-board multicore systems that aims to offer secure, fast, and energy-efficient communication and computation.

## Getting Started

First, install Rust toolchain installer, [`rustup`](https://rustup.rs/).

Then, install the toolchain:

```console
$ ./scripts/install-toolchain
```

Finally, install `xargo`:

```
$ cargo install xargo
```

and `xorriso`:

```
$ dnf install xorriso
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
