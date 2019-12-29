#include "kernel/panic.h"

#include <stdio.h>

void panic(const char *format, ...) {
	printf("\033[91mKernel panic: \033[97m");

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	for (;;);
}
