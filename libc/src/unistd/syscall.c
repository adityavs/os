#include "unistd.h"

void sys_clear() {
	syscall(SYS_CLEAR, 0, 0, 0, 0, 0, 0);
}

void sys_update_cursor() {
	syscall(SYS_UPDATE_CURSOR, 0, 0, 0, 0, 0, 0);
}

int sys_putchar(int c) {
	return syscall(SYS_PUTCHAR, c, 0, 0, 0, 0, 0);
}

int sys_puts(const char *s) {
	return syscall(SYS_PUTS, (int64_t) s, 0, 0, 0, 0, 0);
}

void sys_yield() {
	syscall(SYS_YIELD, 0, 0, 0, 0, 0, 0);
}
