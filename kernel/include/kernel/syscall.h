#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H 1

#include <stdint.h>
#include <unistd.h>

void syscall_init();
int64_t syscall_handler();

#endif
