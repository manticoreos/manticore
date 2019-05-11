#include <stdlib.h>

#include <manticore/syscalls.h>

void abort(void)
{
	exit(1);
}
