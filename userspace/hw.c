int strlen(const char *s) {
	int len = 0;
	while (s[len])
		len++;
	return len;
}

void main() {
	char *vidmem = (char*) 0xB8000;
	char *string = "Hello, World!";
	for (int i = 0; i < strlen(string); i++) {
		vidmem[i * 2] = string[i];
	}
}
