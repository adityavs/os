#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H 1

#include <stdbool.h>
#include <stdint.h>

#define CMM_OFFSET 0x10000000000

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_2MIB 0x200000
#define PAGE_SIZE_1GIB 0x40000000

struct page_table_entry {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t user_accessible : 1;
	uint64_t write_through_caching : 1;
	uint64_t disable_cache : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t huge : 1;
	uint64_t global : 1;
	uint64_t unused : 3;
	uint64_t frame : 40;
	uint64_t unused1 : 11;
	uint64_t no_execute : 1;
} __attribute__ ((packed));

struct page_table {
	struct page_table_entry entry[512];
};

void memory_init();

/*
 * Physical
 */
bool frame_bitmap_check(uint64_t, uint64_t);
void frame_bitmap_set(uint64_t, uint64_t);
void frame_bitmap_unset(uint64_t, uint64_t);
uint64_t frame_alloc(uint64_t);
void frame_free(uint64_t, uint64_t);

/*
 * Virtual
 */
bool virtual_is_used(struct page_table*, uint64_t);

void virtual_map(struct page_table*, uint64_t, uint64_t, int, bool);
void virtual_map_2mib(struct page_table*, uint64_t, uint64_t, int, bool);

void virtual_alloc_to(struct page_table*, uint64_t, int, bool);
void* virtual_alloc(struct page_table*, int, bool);
void virtual_free(struct page_table*, void*, int);

struct page_table* virtual_new();
void virtual_delete(struct page_table*);

/*
 * Heap
 */
struct heap_node {
	uint64_t size : 63;
	uint64_t free :  1;
	struct heap_node* prev;
	struct heap_node* next;
} __attribute__ ((packed));

struct heap {
	uint64_t start;
	uint64_t size;
	struct heap_node *head;
};

void* heap_alloc(uint64_t);
void heap_free(void*);

/*
 * Debug
 */
void print_mmap();
void print_pages(uint64_t);
void print_heap();

#endif
