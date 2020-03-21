#ifndef __MANTICORE_UAPI_SYSCALL_ABI_H
#define __MANTICORE_UAPI_SYSCALL_ABI_H

enum {
	SYS_exit		= 1,
	SYS_wait		= 2,
	SYS_console_print	= 3,
	SYS_subscribe		= 4,
	SYS_getevents		= 5,
	SYS_get_io_queue	= 6,
	SYS_get_config		= 7,
	SYS_acquire		= 8,
};

#endif
