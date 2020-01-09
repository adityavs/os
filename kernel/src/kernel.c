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
	struct task* task1 = task_new(&task1_main);
	struct task* task2 = task_new(&task2_main);

	task_schedule(task2);
	task_schedule(task1);

	// Idle kernel
	for (;;)
		task_switch(true);
}
