/**
 *
 * FIXMEs:
 * - fix clock to match unix timestamp but with 64 bits
 * - integrate interrupts panic and function panic
 * - maybe revisit interrupts not sure possibly
 * 
 * TODOs:
 * - heap
 * - virtual filesystem or other mechanism
 *   maybe only files for now, no need for devices
 *   hopefully I'll figure out a better way in the future
 * - add simple interrupt keyboard handler for early stages
 *   later implement usb driver and keyboard
 *   then trash original keyboard code to keep shit clean
 * - execute programs/multithread
 *   we implement these things simultaneously right?
 * - for now that's it... still a long path away to running a web browser.
 */

#include <stdint.h>

#include "kernel/ata.h"
#include "kernel/bootinfo.h"
#include "kernel/clock.h"
#include "kernel/interrupts.h"
#include "kernel/io.h"
#include "kernel/memory.h"
#include "kernel/panic.h"
#include "kernel/pci.h"
#include "kernel/stdio.h"
#include "kernel/stdlib.h"
#include "kernel/string.h"
#include "kernel/tfs.h"
#include "kernel/tty.h"
#include "kernel/vga.h"

void debug_colors() {
	printf("\033[30mBlack\n");
	printf("\033[31mRed\n");
	printf("\033[32mGreen\n");
	printf("\033[33mYellow\n");
	printf("\033[34mBlue\n");
	printf("\033[35mMagenta\n");
	printf("\033[36mCyan\n");
	printf("\033[37mWhite\n");
	printf("\033[90mBright Black\n");
	printf("\033[91mBright Red\n");
	printf("\033[92mBright Green\n");
	printf("\033[93mBright Yellow\n");
	printf("\033[94mBright Blue\n");
	printf("\033[95mBright Magenta\n");
	printf("\033[96mBright Cyan\n");
	printf("\033[97mBright White\n");
	printf("\033[0m");
}

void debug_mmap() {
	printf("\033[97mMemory Map\n\033[0m");
	struct bootinfo_mmap *mmap = (struct bootinfo_mmap*) BOOTINFO_MMAP_START;
	uint64_t usable = 0;
	for (uint32_t i = 0; i < mmap->entry_count; i++) {
		struct bootinfo_mmap_entry *e = (struct bootinfo_mmap_entry*) &mmap->entry[i];
		if (e->type == 1) {
			printf("\033[32m");
			usable += e->length;
		} else {
			printf("\033[0m");
		}
		printf("  entry[%d]: 0x%x to 0x%x (%d bytes), type=%d\n",
				i, e->base, e->base + e->length, e->length, e->type);
	}
	printf("%u bytes (%d MiB) of usable memory.\n\n", usable, usable / (1024 * 1024));
}

void debug_pages(uint64_t p4addr) {
	struct page_table *p4 = (struct page_table*)
		((uint64_t) CMM_OFFSET + p4addr);
	for (uint64_t p4i = 0; p4i < 512; p4i++) {
		struct page_table_entry *p4e = &p4->entry[p4i];
		if (p4e->present) {
			struct page_table *p3 = (struct page_table*)
				((uint64_t) CMM_OFFSET + p4e->frame * PAGE_SIZE);
			for (uint64_t p3i = 0; p3i < 512; p3i++) {
				struct page_table_entry *p3e = &p3->entry[p3i];
				if (p3e->present) {
					struct page_table *p2 = (struct page_table*)
						((uint64_t) CMM_OFFSET + p3e->frame * PAGE_SIZE);
					for (uint64_t p2i = 0; p2i < 512; p2i++) {
						struct page_table_entry *p2e = &p2->entry[p2i];
						if (p2e->present) {
							if (p2e->huge) {
								printf("0x%x -> 0x%x (2MiB)\n",
										p4i << 39 | p3i << 30 | p2i << 21,
										p2e->frame * PAGE_SIZE_2MIB);
							} else {
								struct page_table *p1 = (struct page_table*)
									((uint64_t) CMM_OFFSET + p2e->frame * PAGE_SIZE);
								for (uint64_t p1i = 0; p1i < 512; p1i++) {
									struct page_table_entry *p1e = &p1->entry[p1i];
									if (p1e->present) {
										printf("0x%x -> 0x%x\n",
												p4i << 39 | p3i << 30 | p2i << 21 | p1i << 12,
												p1e->frame * PAGE_SIZE);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void debug_time() {
	struct time tm;
	get_time(&tm);
	printf("20%d-%d-%d %d:%d:%d\n\n",
			tm.year, tm.month, tm.day,
			tm.hour, tm.minute, tm.second);
}

void debug_tfs() {
	tfs_print_super();
	tfs_print_usage();
	tfs_print_files();
}

void kernel_main() {
	// Video
	vga_init();
	tty_init();
//	debug_colors();

	// Interrupts
	interrupts_init();
	clock_init();
//	debug_time();

	// Memory
//	debug_mmap();
	memory_init();
//	debug_pages(read_cr3());

	// Files
	ata_init();
	tfs_init();
	debug_tfs();

	printf("\033[97m* \033[0mHalting system.");

	asm ("cli");
	for (;;);
}
