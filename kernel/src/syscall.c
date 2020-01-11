#include "kernel/syscall.h"

#include <stddef.h>
#include <stdio.h>

#include "kernel/cpu.h"
#include "kernel/panic.h"
#include "kernel/task.h"
#include "kernel/tty.h"

void (*syscall_handlers[]) = {
	[SYS_YIELD] = task_yield,
	[SYS_EXIT] = task_exit,

	[SYS_CLEAR] = tty_clear,
	[SYS_UPDATE_CURSOR] = tty_cursor_update,
	[SYS_PUTCHAR] = tty_putchar,
	[SYS_PUTS] = tty_puts,
};

void syscall_init() {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
	wrmsr(MSR_STAR, ((size_t) 16 << 48) | ((size_t) 8 << 32));
	wrmsr(MSR_LSTAR, (uint64_t) &syscall_handler);
	wrmsr(MSR_SFMASK, 0);
}
