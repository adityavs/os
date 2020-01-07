#include "stdio.h"

#if defined(__is_libk)
#include <kernel/tty.h>
#else
#include "unistd.h"
#endif

int puts(const char *s) {
#if defined(__is_libk)
	return tty_puts(s);
#else
	return sys_puts(s);
#endif
}
