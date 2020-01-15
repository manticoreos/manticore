#include <manticore/syscalls.h>

long syscall0(long number)
{
        unsigned long ret;
        asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(number)
		: "rcx", "r11", "memory");
        return ret;
}

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

long syscall2(long number, long arg0, long arg1)
{
        unsigned long ret;
        asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(number), "D"(arg0), "S"(arg1)
		: "rcx", "r11", "memory");
        return ret;
}

long syscall4(long number, long arg0, long arg1, long arg2, long arg3)
{
        unsigned long ret;
        asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(number), "D"(arg0), "S"(arg1), "d"(arg2), "c"(arg3)
		: "r11", "memory");
        return ret;
}
