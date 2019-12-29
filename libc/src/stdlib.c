#include "stdlib.h"

#if defined(__is_libk)
#include <kernel/memory.h>
#endif

void *malloc(size_t size) {
#if defined(__is_libk)
	return heap_alloc(size);
#endif
}

void free(void *ptr) {
#if defined(__is_libk)
	heap_free(ptr);
#endif
}
