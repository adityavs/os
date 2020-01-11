#include "stdlib.h"

#include "string.h"

#if defined(__is_libk)
#include <kernel/memory.h>
#endif

void *calloc(size_t size) {
#if defined(__is_libk)
	void *data = heap_alloc(size);
	memset(data, 0, size);
	return data;
#else
	(void) size;
	return NULL;
#endif
}
