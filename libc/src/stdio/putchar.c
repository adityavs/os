#include "stdio.h"

#if defined(__is_libk)
#include <kernel/tty.h>
#else
#include "unistd.h"
#endif

int putchar(int c) {
#if defined(__is_libk)
	return tty_putchar(c);
#else
	return sys_putchar(c);
#endif
}
