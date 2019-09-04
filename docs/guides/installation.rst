Installation
------------

The document describes instructions using which you can install manticore from
source.

- Install the toolchain using: `./scripts/install-toolchain`

After you have toolchain installed you can build Manticore using

`make`

The build system generates a `kernel.iso` image, can be launched under QEMU with:

`$ ./scripts/run kernel.iso`

Manticore Hacker's Guide
------------------------

The guide describes how you can exploit the os implementation. You can install
the cross compiling toolchain with

`./scripts/install-aarch64-toolchain`

Build an AArch64 kernel image, run:

`make ARCH=aarch64`

Run the AArch64 kernel image under QEMU:

`./scripts/run-aarch64`

You can find the [exploitation guide document](https://github.com/tapaswenipathak/manticore/blob/master/HACKING.md#manticore-hackers-guide).
