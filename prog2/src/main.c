#include <stdio.h>
#include <unistd.h>

void main() {
	printf("Second program!\n");
	sys_yield();
	printf("Runs in two parts!\n");
	sys_exit();
}
