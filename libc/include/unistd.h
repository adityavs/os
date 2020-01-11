#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>

enum syscall_number {
	SYS_YIELD, SYS_EXIT,
	SYS_CLEAR, SYS_UPDATE_CURSOR, SYS_PUTCHAR, SYS_PUTS,
};

// Maximum number of syscall arguments: 5
int64_t syscall(enum syscall_number, ...);

static inline void sys_clear() { syscall(SYS_CLEAR); }
static inline void sys_update_cursor() { syscall(SYS_UPDATE_CURSOR); }
static inline int sys_putchar(int c) { return syscall(SYS_PUTCHAR, c); }
static inline int sys_puts(const char *s) { return syscall(SYS_PUTS, (int64_t) s); }
static inline void sys_yield() { syscall(SYS_YIELD); }
static inline void sys_exit() { syscall(SYS_EXIT); }

#endif
