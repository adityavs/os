#include "kernel/stdlib.h"

#include "kernel/memory.h"

void *malloc(size_t size) {
	return heap_alloc(size);
}

void free(void *ptr) {
	heap_free(ptr);
}
