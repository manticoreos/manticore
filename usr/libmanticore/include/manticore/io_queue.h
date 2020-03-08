#ifndef __LIBMANTICORE_IO_QUEUE_H
#define __LIBMANTICORE_IO_QUEUE_H

#include <stddef.h>

#include <manticore/io_queue_abi.h>

int io_submit(io_queue_t queue, void *addr, size_t len);

#endif
