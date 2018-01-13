#include <manticore/syscalls.h>

long syscall1(long number, long arg0)
{
        unsigned long ret;
        asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(number), "D"(arg0)
		: "rcx", "r11", "memory");
        return ret;
}
