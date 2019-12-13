#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <uapi/manticore/syscall_abi.h>

long syscall(int nr, ...);

#endif
