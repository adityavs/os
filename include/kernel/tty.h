#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/vga.h"

enum tty_ansi_state {
	ANSI_ESC,
	ANSI_BRACKET,
	ANSI_ATTR,
	ANSI_ENDVAL,
};

struct tty_ansi_arg {
	int value;
	bool empty;
};

struct tty_ansi_parser {
	enum tty_ansi_state state;
	struct tty_ansi_arg stack[8];
	int index;
};

struct tty_cell {
	uint8_t ch;
	enum vga_color fg : 4;
	enum vga_color bg : 4;
} __attribute__ ((packed));

struct tty {
	struct tty_cell *buffer;
	size_t width, height;
	size_t cursor_x, cursor_y;
	enum vga_color fg, bg;

	struct tty_ansi_parser ansi_parser;
};

void tty_init();
void tty_clear();

void tty_set(size_t, size_t, char, enum vga_color, enum vga_color);

int tty_putchar(int);
int tty_puts(const char*);

void tty_cursor_set(size_t, size_t);
void tty_cursor_update();

#endif
