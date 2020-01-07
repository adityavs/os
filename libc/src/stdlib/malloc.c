#include "stdlib.h"

#if defined(__is_libk)
#include <kernel/memory.h>
#endif

void *malloc(size_t size) {
#if defined(__is_libk)
	return heap_alloc(size);
#else
	(void) size;
	return NULL;
#endif
}
