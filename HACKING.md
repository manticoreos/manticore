# Manticore Hacking Guide

## Debugging with GDB

You can debug Manticore using GDB when the OS is running under QEMU/KVM.

First, start a QEMU instance with GDB remote debugging enabled using the `-gdb` command line option and also pass the `-S` command line option, which causes QEMU not to start the CPU until the user explicitly starts it with GDB:


```
qemu-system-x86_64 -nographic -enable-kvm -cdrom kernel.iso -gdb tcp::1234 -S
```

Once QEMU is running, use the following script to start a GDB session:

```
./scripts/manticore-gdb
```

The GDB session has a hardware breakpoint at `start_kernel`, which is where your debugging session begins.

When setting your own breakpoints, remember to use the `hbreak`, which uses hardware breakpoints and works more reliabily with OS running under QEMU/KVM.
