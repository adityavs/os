#include "string.h"

void* memset(void *dest, int c, size_t n) {
	char *destchar = (char*) dest;
	while (n--)
		*destchar++ = c;
	return dest;
}
