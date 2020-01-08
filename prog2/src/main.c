#include <stdio.h>
#include <unistd.h>

void main() {
	for (;;) {
		printf("Hello!\n");
		sys_yield();
	}
}
