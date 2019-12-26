#include "kernel/string.h"

#include <stdint.h>

void* memcpy(void *dest, const void *src, size_t n) {
	uint8_t *destptr = (uint8_t*) dest;
	uint8_t *srcptr = (uint8_t*) src;
	while (n--) {
		*destptr++ = *srcptr++;
	}
	return dest;
}

void* memset(void *dest, int c, size_t n) {
	uint8_t *destptr = (uint8_t*) dest;
	while (n--) {
		*destptr++ = c;
	}
	return dest;
}
