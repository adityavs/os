#include "string.h"

void* memcpy(void *dest, const void *src, size_t n) {
	char *destchar = (char*) dest;
	char *srcchar = (char*) src;
	while (n--)
		*destchar++ = *srcchar++;
	return dest;
}
