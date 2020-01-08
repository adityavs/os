#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel/ata.h"
#include "kernel/clock.h"
#include "kernel/cpu.h"
#include "kernel/elf.h"
#include "kernel/interrupts.h"
#include "kernel/memory.h"
#include "kernel/syscall.h"
#include "kernel/task.h"
#include "kernel/tfs.h"
#include "kernel/tty.h"
#include "kernel/vga.h"

void task1_main() {
	sysret(get_rsp(), elf_load((struct page_table*) get_cr3(), "/bin/prog1"));
}

void task2_main() {
	sysret(get_rsp(), elf_load((struct page_table*) get_cr3(), "/bin/prog2"));
}	

void task_new(void *func) {
	struct task *task = (struct task*) malloc(sizeof(struct task));
	task->page_map = virtual_new();
	task->kernel_stack = virtual_alloc(task->page_map, 1, 0) + PAGE_SIZE;
	task->user_stack = virtual_alloc(task->page_map, 1, 1) + PAGE_SIZE;
	uint64_t prev_cr3 = get_cr3();
	set_cr3((uint64_t) task->page_map);
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) func;	// rip
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rbp
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rflags
	set_cr3(prev_cr3);
	task_add(task);
}

void kernel_main() {
	// Video
	vga_init();
	tty_init();

	// Interrupts
	interrupts_init();
	clock_init();

	// Memory
	memory_init();

	// Files
	ata_init();
	tfs_init();

	// Userspace
	syscall_init();
	task_init();

	// More ugly code. But this time we got a _very_ basic
	// scheduler. Also very simple cooperative multitasking
	task_new(&task1_main);
	task_new(&task2_main);

	// Idle kernel
	for (int i = 0; 1; i++)
		task_reschedule();
}
