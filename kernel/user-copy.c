#include <kernel/user-access.h>

#include <kernel/errno.h>

#include <arch/vmem.h>

int raw_copy_from_user(void *dest, const void __user *src, size_t len);

int copy_from_user(void *dest, const void __user *src, size_t len)
{
	/* FIXME: Make this check more strict by looking at process virtual
		  memory limits.  */
	if ((unsigned long)src >= (unsigned long)KERNEL_VMA) {
		return -EFAULT;
	}
	return raw_copy_from_user(dest, src, len);
}

int raw_strncpy_from_user(void *dest, const void __user *src, size_t len);

int strncpy_from_user(void *dest, const void __user *src, size_t len)
{
	/* FIXME: Make this check more strict by looking at process virtual
		  memory limits.  */
	if ((unsigned long)src >= (unsigned long)KERNEL_VMA) {
		return -EFAULT;
	}
	return raw_strncpy_from_user(dest, src, len);
}
