#include "kernel/memory.h"

#include <stdio.h>
#include <string.h>

#include "kernel/bootinfo.h"
#include "kernel/io.h"
#include "kernel/panic.h"

void physical_init(uint64_t);
void virtual_init(uint64_t);
void heap_init(uint64_t);
void memory_init() {
	// Detecting memory
	uint64_t total_memory = 0x100000;
	struct bootinfo_mmap *mmap = (struct bootinfo_mmap*) BOOTINFO_MMAP_START;
	for (uint64_t i = 0; i < mmap->entry_count; i++) {
		struct bootinfo_mmap_entry *e = (struct bootinfo_mmap_entry*) &mmap->entry[i];
		if (e->base == 0x100000 && e->type == 1)
			total_memory += e->length;
	}
	if (total_memory == 0x100000)
		panic("no memory after 0x100000");

	physical_init(total_memory);
	virtual_init(total_memory);
	heap_init(0x100000);
}

/*
 * Physical
 */
uint64_t frame_count;
uint64_t frame_bitmap_size;
uint8_t *frame_bitmap;

void physical_init(uint64_t total_memory) {
	// Setting up frame bitmap at 0x100000
	frame_count = (total_memory + PAGE_SIZE - 1) / PAGE_SIZE;
	frame_bitmap_size = (frame_count + 7) / 8;
	frame_bitmap = (uint8_t*) 0x100000;
	memset(frame_bitmap, 0, frame_bitmap_size);
	frame_bitmap_set(0, (0x100000 + frame_bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);
}

bool frame_bitmap_check(uint64_t index, uint64_t length) {
	for (uint64_t i = 0; i < length; i++)
		if (frame_bitmap[(index + i) / 8] & (1 << (7 - ((index + i) % 8))))
			return true;
	return false;
}

void frame_bitmap_set(uint64_t index, uint64_t length) {
	for (uint64_t i = 0; i < length; i++)
		frame_bitmap[(index + i) / 8] |= (1 << (7 - ((index + i) % 8)));
}

void frame_bitmap_unset(uint64_t index, uint64_t length) {
	for (uint64_t i = 0; i < length; i++)
		frame_bitmap[(index + i) / 8] &= ~(1 << (7 - ((index + i) % 8)));
}

uint64_t frame_alloc(uint64_t length) {
	for (uint64_t i = 0; i < frame_count; i++) {
		if (!frame_bitmap_check(i, length)) {
			frame_bitmap_set(i, length);
			return i;
		}
	}

	panic("frame_alloc() failed. ran out of memory?\n");
	return -1;
}

void frame_free(uint64_t index, uint64_t length) {
	frame_bitmap_unset(index, length);
}

/*
 * Virtual
 */
#define P4_INDEX(vaddr) (((vaddr) >> 39) & 0x1FF)
#define P3_INDEX(vaddr) (((vaddr) >> 30) & 0x1FF)
#define P2_INDEX(vaddr) (((vaddr) >> 21) & 0x1FF)
#define P1_INDEX(vaddr) (((vaddr) >> 12) & 0x1FF)

struct page_table *kernel_p4;
uint64_t page_offset = 0;

void virtual_init(uint64_t total_memory) {
	// Allocate a page for kernel's P4
	kernel_p4 = (struct page_table*) (frame_alloc(1) * PAGE_SIZE);
	memset(kernel_p4, 0, PAGE_SIZE);

	// Map the complete physical space to +1TiB because why the fuck not
	virtual_map_2mib(kernel_p4, CMM_OFFSET, 0, total_memory / PAGE_SIZE_2MIB);

	// Identity map from 0 to what we've used
	virtual_map(kernel_p4, 0, 0, (uint64_t) kernel_p4 / PAGE_SIZE, 0);

	// Move kernel's P4 address to cr3
	virtual_set_p4(kernel_p4);
	page_offset = CMM_OFFSET;
}

void virtual_map(struct page_table *p4, uint64_t vaddr, uint64_t paddr, uint64_t page_count,
		int8_t user) {
	for (uint64_t page = 0; page < page_count; page++) {
		struct page_table_entry *p4e = &((struct page_table*)
				((uint64_t) p4 + page_offset))->entry[P4_INDEX(vaddr)];
		if (!p4e->present) {
			p4e->present = 1;
			p4e->writable = 1;
			p4e->user_accessible = user;
			p4e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (page_offset + (p4e->frame * PAGE_SIZE)), 0, PAGE_SIZE);
		}
		if (user) p4e->user_accessible = 1;
		struct page_table *p3 = (struct page_table*) (uint64_t)
			(page_offset + (p4e->frame * PAGE_SIZE));
		struct page_table_entry *p3e = &p3->entry[P3_INDEX(vaddr)];
		if (!p3e->present) {
			p3e->present = 1;
			p3e->writable = 1;
			p3e->user_accessible = user;
			p3e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (page_offset + (p3e->frame * PAGE_SIZE)), 0, PAGE_SIZE);
		}
		if (user) p3e->user_accessible = 1;
		struct page_table *p2 = (struct page_table*) (uint64_t)
			(page_offset + (p3e->frame * PAGE_SIZE));
		struct page_table_entry *p2e = &p2->entry[P2_INDEX(vaddr)];
		if (!p2e->present) {
			p2e->present = 1;
			p2e->writable = 1;
			p2e->user_accessible = user;
			p2e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (page_offset + (p2e->frame * PAGE_SIZE)), 0, PAGE_SIZE);
		}
		if (user) p2e->user_accessible = 1;
		struct page_table *p1 = (struct page_table*) (uint64_t)
			(page_offset + (p2e->frame * PAGE_SIZE));
		struct page_table_entry *p1e = &p1->entry[P1_INDEX(vaddr)];
		if (!p1e->present) {
			p1e->present = 1;
			p1e->writable = 1;
			p1e->user_accessible = user;
			p1e->frame = paddr / PAGE_SIZE;
		}
		vaddr += PAGE_SIZE;
		paddr += PAGE_SIZE;
	}
}

void virtual_map_2mib(struct page_table *p4, uint64_t vaddr, uint64_t paddr, uint64_t count) {
	for (uint64_t page = 0; page < count; page++) {
		struct page_table_entry *p4e = &p4->entry[P4_INDEX(vaddr)];
		if (!p4e->present) {
			p4e->present = 1;
			p4e->writable = 1;
			p4e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (p4e->frame * PAGE_SIZE), 0, PAGE_SIZE);
		}
		struct page_table *p3 = (struct page_table*) (uint64_t) (p4e->frame * PAGE_SIZE);
		struct page_table_entry *p3e = &p3->entry[P3_INDEX(vaddr)];
		if (!p3e->present) {
			p3e->present = 1;
			p3e->writable = 1;
			p3e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (p3e->frame * PAGE_SIZE), 0, PAGE_SIZE);
		}
		struct page_table *p2 = (struct page_table*) (uint64_t) (p3e->frame * PAGE_SIZE);
		struct page_table_entry *p2e = &p2->entry[P2_INDEX(vaddr)];
		if (!p2e->present) {
			p2e->present = 1;
			p2e->writable = 1;
			p2e->huge = 1;
			p2e->frame = paddr / PAGE_SIZE;
		}
		vaddr += PAGE_SIZE_2MIB;
		paddr += PAGE_SIZE_2MIB;
	}
}

void virtual_map_1gib(struct page_table *p4, uint64_t vaddr, uint64_t paddr, uint64_t count) {
	for (uint64_t page = 0; page < count; page++) {
		struct page_table_entry *p4e = &p4->entry[P4_INDEX(vaddr)];
		if (!p4e->present) {
			p4e->present = 1;
			p4e->writable = 1;
			p4e->frame = frame_alloc(1);
			memset((void*) (uint64_t) (p4e->frame * PAGE_SIZE), 0, PAGE_SIZE);
		}
		struct page_table *p3 = (struct page_table*) (uint64_t) (p4e->frame * PAGE_SIZE);
		struct page_table_entry *p3e = &p3->entry[P3_INDEX(vaddr)];
		if (!p3e->present) {
			p3e->present = 1;
			p3e->writable = 1;
			p3e->huge = 1;
			p3e->frame = paddr / PAGE_SIZE;
		}
		vaddr += PAGE_SIZE_1GIB;
		paddr += PAGE_SIZE_1GIB;
	}
}

bool virtual_is_used(struct page_table *p4, uint64_t vaddr) {
	struct page_table_entry *p4e = &((struct page_table*)
			((uint64_t) p4 + page_offset))->entry[P4_INDEX(vaddr)];
	if (!p4e->present) return false;
	struct page_table *p3 = (struct page_table*) (uint64_t)
		(page_offset + (p4e->frame * PAGE_SIZE));
	struct page_table_entry *p3e = &p3->entry[P3_INDEX(vaddr)];
	if (!p3e->present) return false;
	struct page_table *p2 = (struct page_table*) (uint64_t)
		(page_offset + (p3e->frame * PAGE_SIZE));
	struct page_table_entry *p2e = &p2->entry[P2_INDEX(vaddr)];
	if (!p2e->present) return false;
	struct page_table *p1 = (struct page_table*) (uint64_t)
		(page_offset + (p2e->frame * PAGE_SIZE));
	struct page_table_entry *p1e = &p1->entry[P1_INDEX(vaddr)];
	return p1e->present;
}

void virtual_alloc_to(struct page_table *p4, uint64_t vaddr, uint64_t page_count,
		int8_t user) {
	for (uint64_t page = 0; page < page_count; page++)
		virtual_map(p4, vaddr + PAGE_SIZE * page, frame_alloc(1) * PAGE_SIZE, 1, user);
}

void* virtual_alloc(struct page_table *p4, uint64_t page_count, int8_t user) {
	uint64_t base = 0, length = 0;
	for (uint64_t vaddr = 0; vaddr < CMM_OFFSET; vaddr += PAGE_SIZE) {
		if (!virtual_is_used(p4, vaddr)) {
			if (++length == page_count) {
				virtual_alloc_to(p4, base, page_count, user);
				return (void*) base;
			}
		} else {
			base = vaddr + PAGE_SIZE;
			length = 0;
		}
	}
	return NULL;
}

struct page_table* virtual_new() {
	struct page_table *new_p4 = (struct page_table*) (frame_alloc(1) * PAGE_SIZE);
	memcpy((void*) (uint64_t) new_p4 + page_offset,
			(void*) (uint64_t) kernel_p4 + page_offset,
			PAGE_SIZE);
	return new_p4;
}

/*
 * Heap
 */
struct heap heap;

void heap_init(uint64_t size) {
	heap.size = size;
	heap.start = (uint64_t) virtual_alloc(kernel_p4, heap.size / PAGE_SIZE, 0);

	heap.head = (struct heap_node*) heap.start;
	heap.head->size = heap.size - sizeof(struct heap_node);
	heap.head->free = 1;
	heap.head->prev = NULL;
	heap.head->next = NULL;
}

void *heap_alloc(uint64_t size) {
	uint64_t addr = heap.start + sizeof(struct heap_node);
	struct heap_node *node = heap.head;
	while (node) {
		if (node->free) {
			if (node->size == size) {
				node->free = 0;
				return (void*) addr;
			} else if (node->size > size + sizeof(struct heap_node)) {
				struct heap_node *next = (struct heap_node*) (addr + size);
				next->size = node->size - sizeof(struct heap_node) - size;
				next->free = 1;
				next->prev = node;
				next->next = node->next;

				node->size = size;
				node->free = 0;
				node->next = next;
				return (void*) addr;
			}
		}
		addr += sizeof(struct heap_node) + node->size;
		node = node->next;
	}

	panic("heap_alloc(%d) failed.\n", size);
	return NULL;
}

void heap_free(void *addr) {
	struct heap_node *node = (struct heap_node*) (addr - sizeof(struct heap_node));
	node->free = 1;

	if (node->next != NULL) {
		struct heap_node *next = node->next;
		if (next->free) {
			node->size += sizeof(struct heap_node) + next->size;
			if (next->next != NULL)
				next->next->prev = node;
			node->next = next->next;
		}
	}
	if (node->prev != NULL) {
		struct heap_node *prev = node->prev;
		if (prev->free) {
			prev->size += sizeof(struct heap_node) + node->size;
			if (node->next != NULL)
				node->next->prev = prev;
			prev->next = node->next;
		}
	}
}

/*
 * Debug
 */
void print_mmap() {
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

void print_page(struct page_table_entry *pte, uint64_t vaddr, uint8_t entry_level) {
	printf("%016x: %016x %c%c%c%c%c%c%c%c%c\n",
			vaddr,
			pte->frame * PAGE_SIZE,
			pte->no_execute ? 'X' : '-',
			pte->global ? 'G' : '-',
			pte->huge ? (entry_level == 2 ? 'G' : 'M') : '-',
			pte->dirty ? 'D' : '-',
			pte->accessed ? 'A' : '-',
			pte->disable_cache ? 'C' : '-',
			pte->write_through_caching ? 'T' : '-',
			pte->user_accessible ? 'U' : '-',
			pte->writable ? 'W' : '-');
}

void print_pages(uint64_t p4addr) {
    struct page_table *p4 = (struct page_table*)
        ((uint64_t) CMM_OFFSET + p4addr);
    for (uint64_t p4i = 0; p4i < 512; p4i++) {
        struct page_table_entry *p4e = &p4->entry[p4i];
        if (!p4e->present)
            continue;
        struct page_table *p3 = (struct page_table*)
            ((uint64_t) CMM_OFFSET + p4e->frame * PAGE_SIZE);
        for (uint64_t p3i = 0; p3i < 512; p3i++) {
            struct page_table_entry *p3e = &p3->entry[p3i];
            if (!p3e->present)
                continue;
            if (p3e->huge) {
            	print_page(p3e, p4i << 39 | p3i << 30, 2);
				continue;
            }
            struct page_table *p2 = (struct page_table*)
                ((uint64_t) CMM_OFFSET + p3e->frame * PAGE_SIZE);
            for (uint64_t p2i = 0; p2i < 512; p2i++) {
                struct page_table_entry *p2e = &p2->entry[p2i];
                if (!p2e->present)
                    continue;
                if (p2e->huge) {
                    print_page(p2e, p4i << 39 | p3i << 30 | p2i << 21, 1);
                    continue;
                }
                struct page_table *p1 = (struct page_table*)
                    ((uint64_t) CMM_OFFSET + p2e->frame * PAGE_SIZE);
                for (uint64_t p1i = 0; p1i < 512; p1i++) {
                    struct page_table_entry *p1e = &p1->entry[p1i];
                    if (!p1e->present)
                        continue;
                    print_page(p1e, p4i << 39 | p3i << 30 | p2i << 21 | p1i << 12, 0);
                }
            }
        }
    }
}

void print_heap() {
	struct heap_node *node = heap.head;
	while (node) {
		printf("%s%d,%d%s", node->prev == NULL ? "{" : "(",
				node->size,
				node->free,
				node->next == NULL ? "}\n" : "), ");
		node = node->next;
	}
}
