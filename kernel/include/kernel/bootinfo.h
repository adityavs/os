#ifndef _KERNEL_BOOTINFO_H
#define _KERNEL_BOOTINFO_H 1

#define BOOTINFO_MMAP_START 0x500

struct bootinfo_mmap_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t acpi;
};

struct bootinfo_mmap {
	uint64_t entry_count;
	struct bootinfo_mmap_entry entry[];
};

#endif
