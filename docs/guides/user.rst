Manticore
----------

The Manticore source tree has some example applications showing how to run
applications on top of the OS. Say, once `kernel.elf` is built, you can build
the lossy/untrusted echod protocol echo server with

`$ make -C usr/echod`

You might install QEMU for virtually running multiple Operating Systems. After
installing you can launch manticore under QEMU with

`$ ./scripts/run usr/echod/echod.iso`
