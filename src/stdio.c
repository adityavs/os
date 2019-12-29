#include "kernel/stdio.h"

#include <stdarg.h>
#include <stdint.h>

#include "kernel/ctype.h"
#include "kernel/tty.h"

static char* itoa(uint64_t i, int base, int pad, char padchar) {
	static const char* rep = "0123456789ABCDEF";
	static char buffer[65] = {0};
	char *ptr = &buffer[64];
	*ptr = '\0';
	do {
		*--ptr = rep[i % base];
		if (pad) pad--;
		i /= base;
	} while (i != 0);
	while (pad--) {
		*--ptr = padchar;
	}
	return ptr;
}

int printf(const char *format, ...) {
	int written = 0;
	va_list args;
	va_start(args, format);
	written = vprintf(format, args);
	va_end(args);
	return written;
}

int sprintf(char *buffer, const char *format, ...) {
	int written = 0;
	va_list args;
	va_start(args, format);
	written = vsprintf(buffer, format, args);
	va_end(args);
	return written;
}

int vprintf(const char *format, va_list args) {
	static char buffer[1024] = { 0 };
	int written = vsprintf(buffer, format, args);
	puts(buffer);
	return written;
}

int vsprintf(char *buffer, const char *format, va_list args) {
	uint64_t written = 0;
	uint8_t pad = 0;
	char padchar = ' ';

	while (*format != '\0') {
		if (*format == '%' || pad != 0) {
			if (*format == '%')
				format++;
			if (*format == '%') {
				buffer[written++] = '%';
				format++;
			} else if (*format == 'c') {
				char c = (char) va_arg(args, int);
				buffer[written++] = c;
				format++;
			} else if (*format == 's') {
				char *s = va_arg(args, char*);
				while (*s)
					buffer[written++] = *s++;
				format++;
			} else if (*format == 'd') {
				int32_t d = va_arg(args, int64_t);
				if (d < 0) {
					d = -d;
					buffer[written++] = '-';
				}
				char *s = itoa(d, 10, pad, padchar);
				pad = 0;
				while (*s)
					buffer[written++] = *s++;
				format++;
			} else if (*format == 'u') {
				char *s = itoa(va_arg(args, uint64_t), 10, pad, padchar);
				pad = 0;
				while (*s)
					buffer[written++] = *s++;
				format++;
			} else if (*format == 'x') {
				char *s = itoa(va_arg(args, uint64_t), 16, pad, padchar);
				pad = 0;
				while (*s)
					buffer[written++] = *s++;
				format++;
			} else if (isdigit(*format)) {
				if (*format == '0') {
					padchar = '0';
					format++;
				} else {
					padchar = ' ';
				}
				while (isdigit(*format)) {
					pad *= 10;
					pad += (*format++ - '0');
				}
			}
		} else {
			buffer[written++] = *format++;
		}
	}

	buffer[written] = 0;
	return written;
}

int putchar(int c) {
	return tty_putchar(c);
}

int puts(const char *s) {
	return tty_puts(s);
}
