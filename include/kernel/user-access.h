//
// Userspace access functions (i.e. data copies between kernel and userspace).
//
#ifndef KERNEL_USER_ACCESS_H
#define KERNEL_USER_ACCESS_H

#include <stddef.h>

// TODO: userspace pointer annotation
#define __user

/// Copy memory region from userspace to a kernel buffer.
int copy_from_user(void *dest, const void __user *src, size_t len);

#endif
