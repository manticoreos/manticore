# Manticore Hacker's Guide

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

## Running kernel test suite

The kernel has an internal test suite that runs in kernel space. The test suite is not compiled by default, but it built and run using the following commands:

```
make TEST=1 && ./scripts/run.sh
```

## Debugging with GDB

You can debug Manticore using GDB when the OS is running under QEMU/KVM.

First, start a QEMU instance with GDB remote debugging enabled using the `-gdb` command line option and also pass the `-S` command line option, which causes QEMU not to start the CPU until the user explicitly starts it with GDB:

```
./scripts/run --debug kernel.iso
```

Once QEMU is running, use the following script to start a GDB session:

```
./scripts/manticore-gdb
```

The GDB session has a hardware breakpoint at `start_kernel`, which is where your debugging session begins.

When setting your own breakpoints, remember to use the `hbreak`, which uses hardware breakpoints and works more reliably with OS running under QEMU/KVM.

## Tracing network traffic

You can dump network traffic to a pcap file with the `--network-dump FILENAME` command line option to `scripts/run`.
Please note that the option currently only works with `user` networking.

To dump network traffic, start the VM with:

```
./scripts/run --network=user --network-dump=net.pcap kernel.iso
```

To inspect the pcap file, run:

```
tshark -r net.pcap -V
```
