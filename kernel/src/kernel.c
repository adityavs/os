#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel/ata.h"
#include "kernel/bootinfo.h"
#include "kernel/clock.h"
#include "kernel/elf.h"
#include "kernel/interrupts.h"
#include "kernel/io.h"
#include "kernel/memory.h"
#include "kernel/panic.h"
#include "kernel/pci.h"
#include "kernel/syscall.h"
#include "kernel/tfs.h"
#include "kernel/tty.h"
#include "kernel/vga.h"

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

	// This code is really ugly and still needs a lot of refactoring
	// But I'm gonna take a small break from this project so I decided
	// to push these changes as-is. I still need to create some process
	// structs and a scheduler. Also fix interrupts in usermode.
	struct page_table *p4 = virtual_new();
	uint64_t entry = elf_load(p4, "/bin/hw");
	uint64_t user_stack = (uint64_t) virtual_alloc(p4, 4, 1) + PAGE_SIZE * 4;
	uint64_t kernel_stack = (uint64_t) virtual_alloc(p4, 1, 0) + PAGE_SIZE * 1;

	virtual_set_p4(p4);
	extern uint32_t tss;
	(&tss)[1] = kernel_stack & 0xFFFFFFFF;
	(&tss)[2] = kernel_stack >> 32;
	asm ("mov %0, %%rsp" : : "r" (user_stack));
	asm ("rex.w sysret" : : "c" (entry));

	// Legacy code
	printf("\033[97m* \033[0mHalting system.");
	asm ("cli");
	for (;;);
}
