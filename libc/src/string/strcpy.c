#include "string.h"

void strcpy(char *dest, const char *src) {
	strncpy(dest, src, strlen(src));
}
