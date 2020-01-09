#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>

#define SYS_CLEAR 0
#define SYS_UPDATE_CURSOR 1
#define SYS_PUTCHAR 2
#define SYS_PUTS 3
#define SYS_YIELD 4
#define SYS_EXIT 5

int64_t syscall(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t);

static inline void sys_clear() { syscall(SYS_CLEAR, 0, 0, 0, 0, 0, 0); }
static inline void sys_update_cursor() { syscall(SYS_UPDATE_CURSOR, 0, 0, 0, 0, 0, 0); }
static inline int sys_putchar(int c) { return syscall(SYS_PUTCHAR, c, 0, 0, 0, 0, 0); }
static inline int sys_puts(const char *s) { return syscall(SYS_PUTS, (int64_t) s, 0, 0, 0, 0, 0); }
static inline void sys_yield() { syscall(SYS_YIELD, 0, 0, 0, 0, 0, 0); }
static inline void sys_exit() { syscall(SYS_EXIT, 0, 0, 0, 0, 0, 0); }

#endif
