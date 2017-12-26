# Manticore

[![Build Status](https://semaphoreci.com/api/v1/projects/3ee7d6de-333a-4b15-afbc-065e3825778b/1298917/badge.svg)](https://semaphoreci.com/manticore/manticore)

Manticore is an experimental OS for edge computing that has the following, sometimes conflicting, goals:

* **Security** - The OS provides a foundation for secure computing. It's critical that the OS leverages hardware protection capabilities, provides minimal and safe system call interface to minimize attack surface, and is itself implemented in high-level language to minimize exploitable bugs.
* **Low latency** - One of the main motivations of edge computing is to provide lower latency than an equivalent cloud-based service would have. It's important that the OS itself has low overheads and provides interfaces that support low-latency applications.
* **Energy efficiency** - Communications technology is forecast to consume around 20% of global electricity by 2030, or as much as 50% in the worst case (Andrae and Edler, 2015)! It's therefore important that the OS provides interfaces that enable energy efficient computing.

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

## References

Anders S. G. Andrae and Tomas Edler. 2015. On Global Electricity Usage of Communication Technology: Trends to 2030. In _Challenges_, 6(1):117–157, 2015. DOI: http://dx.doi.org/10.3390/challe6010117