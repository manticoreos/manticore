#include <assert.h>
#include <errno.h>

#include <manticore/syscalls.h>

int main(int argc, char *argv[])
{
	assert(console_print("hello\n", 6) == 6);
	assert(console_print((void *)0xdeadbeef, 1) == -EFAULT);
	assert((int)console_print((void *)0xffffffff80000000, 1) == (int)-EFAULT);

	exit(0);
}
