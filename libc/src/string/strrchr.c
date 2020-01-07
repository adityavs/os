#include "string.h"

char* strrchr(const char *s, int c) {
	const char *p = NULL;
	while (*s) {
		if (*s == (char) c)
			p = s;
		s++;
	}
	return (char*) p;
}
