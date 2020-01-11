#include <kernel/user-access.h>

#include <kernel/errno.h>

#include <arch/vmem.h>

extern int __memcpy_user_safe(void __user *dest, const void *src, size_t len);

int memcpy_to_user(void __user *dest, const void *src, size_t len)
{
	/* FIXME: Make this check more strict by looking at process virtual
		  memory limits.  */
	if ((unsigned long)dest >= (unsigned long)KERNEL_VMA) {
		return -EFAULT;
	}
	return __memcpy_user_safe(dest, src, len);
}

int memcpy_from_user(void *dest, const void __user *src, size_t len)
{
	/* FIXME: Make this check more strict by looking at process virtual
		  memory limits.  */
	if ((unsigned long)src >= (unsigned long)KERNEL_VMA) {
		return -EFAULT;
	}
	return __memcpy_user_safe(dest, src, len);
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
