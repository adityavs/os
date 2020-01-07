#include "string.h"

void strncpy(char *dest, const char *src, size_t n) {
	while (n--) {
		if (!*src)
			break;
		*dest++ = *src++;
	}
	*dest = '\0';
}
