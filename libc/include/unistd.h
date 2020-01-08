#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>

#define SYS_CLEAR 0
#define SYS_UPDATE_CURSOR 1
#define SYS_PUTCHAR 2
#define SYS_PUTS 3
#define SYS_YIELD 4

int64_t syscall(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t);

void sys_clear();
void sys_update_cursor();
int sys_putchar(int);
int sys_puts(const char*);
void sys_yield();

#endif
