#include <stdio.h>
#include <unistd.h>

void main() {
	for (;;) {
		printf("Hi!\n");
		sys_yield();
	}
}
