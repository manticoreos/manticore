#ifndef __MANTICORE_KERNEL_DEVICE_H
#define __MANTICORE_KERNEL_DEVICE_H

#include <stddef.h>

int device_get_config(const char *dev_name, int opt, void *buf, size_t len);

#endif
