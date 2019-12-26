#include "kernel/panic.h"

#include "kernel/stdio.h"

void panic(const char *s) {
	printf("\033[91mKernel panic: \033[97m%s\n", s);
	for (;;);
}
