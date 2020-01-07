#include "stdlib.h"

#if defined(__is_libk)
#include <kernel/memory.h>
#endif

void free(void *ptr) {
#if defined(__is_libk)
	heap_free(ptr);
#else
	(void) ptr;
#endif
}
