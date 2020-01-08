#include "kernel/syscall.h"

#include <stddef.h>
#include <stdio.h>

#include "kernel/cpu.h"
#include "kernel/panic.h"
#include "kernel/task.h"
#include "kernel/tty.h"

uint64_t syscall_handlers[256] = { 0 };

void sys_yield() {
	task_reschedule();
}

void sys_clear() {
	tty_clear();
}

void sys_cursor_update() {
	tty_cursor_update();
}

int sys_putchar(int c) {
	return tty_putchar(c);
}

int sys_puts(const char *s) {
	return tty_puts(s);
}

void syscall_init() {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
	wrmsr(MSR_STAR, ((size_t) 16 << 48) | ((size_t) 8 << 32));
	wrmsr(MSR_LSTAR, (uint64_t) &syscall_handler);
	wrmsr(MSR_SFMASK, 0);

	syscall_handlers[0] = (uint64_t) &sys_clear;
	syscall_handlers[1] = (uint64_t) &sys_cursor_update;
	syscall_handlers[2] = (uint64_t) &sys_putchar;
	syscall_handlers[3] = (uint64_t) &sys_puts;
	syscall_handlers[4] = (uint64_t) &sys_yield;
}
