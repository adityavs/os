#include "kernel/string.h"

#include <stdint.h>

#include "kernel/stdlib.h"

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

void strncpy(char *dest, const char *src, size_t n) {
	while (n--) {
		if (!*src)
			break;
		*dest++ = *src++;
	}
	*dest = '\0';
}

void strcpy(char *dest, const char *src) {
	strncpy(dest, src, strlen(src));
}

int strcmp(const char *a, const char *b) {
	while (*a && (*a == *b)) {
		a++;
		b++;
	}
	return *((const unsigned char*) a)
		 - *((const unsigned char*) b);
}

char *strdup(const char *s) {
	char *dest = malloc(strlen(s) + 1);
	if (dest == NULL)
		return NULL;
	strcpy(dest, s);
	return dest;
}

size_t strlen(const char *s) {
	size_t len = 0;
	while (*s++)
		len++;
	return len;
}

char* strrchr(const char *s, int c) {
	const char *p = NULL;
	while (*s) {
		if (*s == (char) c)
			p = s;
		s++;
	}
	return (char*) p;
}
