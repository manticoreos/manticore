//
// Userspace access functions (i.e. data copies between kernel and userspace).
//
#ifndef KERNEL_USER_ACCESS_H
#define KERNEL_USER_ACCESS_H

#include <stddef.h>

// TODO: userspace pointer annotation
#define __user

/// Copy memory region from userspace to a kernel buffer.
int memcpy_from_user(void *dest, const void __user *src, size_t len);

/// Copy a NULL-terminated string from userspace to a kernel buffer.
int strncpy_from_user(void *dest, const void __user *src, size_t len);

/// Copy memory region from kernel space to an userspace buffer.
int memcpy_to_user(void __user *dest, const void *src, size_t len);

#endif
