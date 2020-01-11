#include <stdio.h>
#include <string.h>
#include <unistd.h>

void main() {
	printf("%d\n", printf("printf(\"Hello, World!\")=") - strlen("printf(\"\")="));
	sys_exit();
}
