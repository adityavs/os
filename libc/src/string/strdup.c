#include "string.h"

#include "stdlib.h"

char *strdup(const char *s) {
	char *dest = malloc(strlen(s) + 1);
	if (dest == NULL)
		return NULL;
	strcpy(dest, s);
	return dest;
}
