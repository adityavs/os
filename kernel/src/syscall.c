#include "kernel/syscall.h"

#include <stddef.h>
#include <stdio.h>

#include "kernel/cpu.h"
#include "kernel/panic.h"
#include "kernel/task.h"
#include "kernel/tty.h"

uint64_t syscall_handlers[256] = { 0 };

void sys_clear_handler() {
	tty_clear();
}

void sys_update_cursor_handler() {
	tty_cursor_update();
}

int sys_putchar_handler(int c) {
	return tty_putchar(c);
}

int sys_puts_handler(const char *s) {
	return tty_puts(s);
}

void sys_yield_handler() {
	task_switch(true);
}

void sys_exit_handler() {
	task_switch(false);
}

void syscall_init() {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
	wrmsr(MSR_STAR, ((size_t) 16 << 48) | ((size_t) 8 << 32));
	wrmsr(MSR_LSTAR, (uint64_t) &syscall_handler);
	wrmsr(MSR_SFMASK, 0);

	syscall_handlers[SYS_CLEAR] = (uint64_t) &sys_clear_handler;
	syscall_handlers[SYS_UPDATE_CURSOR] = (uint64_t) &sys_update_cursor_handler;
	syscall_handlers[SYS_PUTCHAR] = (uint64_t) &sys_putchar_handler;
	syscall_handlers[SYS_PUTS] = (uint64_t) &sys_puts_handler;
	syscall_handlers[SYS_YIELD] = (uint64_t) &sys_yield_handler;
	syscall_handlers[SYS_EXIT] = (uint64_t) &sys_exit_handler;
}
