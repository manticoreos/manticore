Manticore Architecture
----------------------

Manticore is an research implementation of parakernels. The main implementation
requires implementing a kernel space, user space, NIC and process space for threads
impmentation.

Arch contains machine architecture specific code. Drivers are the device drivers
having virtIO device drivers and PICe device drivers. Kernel directory has kernel
space implementation like process schedule, thread creation, interaction and
system calls. lib has support libraries. mm is the memory management handling having
virtual memory manager and kernel dynamic memory allocator. usr directory contains
user space libraries and example applications.

The OS differs from a certain kernel as the OS will have a application layer for
interacting with the kernel.

What each directory holds?
--------------------------

- echod: echod is the UDP echo server. UDP is lossy protocol. There is no surety 
  of message being uncorrupted but the implementation and transfer is fast.

- init: init is an implementation of exit(0) function testing a manticore systemcall.

- liblinux: liblinux is an implementation of arpa and arch socket implementation.

- libmanticore: libmanticore implements kernel level system calls.

Supported Hardware
-------------------

- Legacy-free PC with a 64-bit x86 processor
  - xAPIC2 interrupt controller
  - MSI-X interrupt delivery
  - PCIe 3.0 bus
- VirtIO network device
