#include "kernel/tty.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "kernel/cpu.h"

struct tty tty = {
	.buffer = (struct tty_cell*) 0xB8000,
	.width = 80, .height = 25,
	.cursor_x = 0, .cursor_y = 0,
	.fg = VGA_WHITE, .bg = VGA_BLACK,
	.ansi_parser = {
		.state = ANSI_ESC,
	},
};

void tty_clear() {
	tty.cursor_x = 0;
	tty.cursor_y = 0;
	for (size_t i = 0; i < tty.width * tty.height; i++) {
		tty.buffer[i].ch = ' ';
		tty.buffer[i].fg = tty.fg;
		tty.buffer[i].bg = tty.bg;
	}
}

void tty_scroll(size_t lines) {
	if (lines >= tty.height) {
		tty_clear();
		return;
	}
	memcpy(tty.buffer,
		   tty.buffer + tty.width * lines,
		   tty.width * (tty.height - lines) * sizeof(uint16_t));
	for (size_t i = tty.width * (tty.height - lines); i < tty.width * tty.height; i++) {
		tty.buffer[i].ch = ' ';
		tty.buffer[i].fg = tty.fg;
		tty.buffer[i].bg = tty.bg;
	}
	tty.cursor_y -= lines;
}

void tty_set(size_t x, size_t y, char c, enum vga_color fg, enum vga_color bg) {
	struct tty_cell *cell = &tty.buffer[y * tty.width + x];
	cell->ch = c;
	cell->fg = fg;
	cell->bg = bg;
}

int tty_append(int ch) {
	if (ch == '\n') {
		tty.cursor_x = 0;
		if (++tty.cursor_y >= tty.height) {
			tty_scroll(1);
		}
	} else if (ch == '\b') {
		if (tty.cursor_x-- == 0) {
			if (tty.cursor_y > 0) {
				tty.cursor_x = tty.width - 1;
				tty.cursor_y--;
			} else {
				tty.cursor_x = 0;
			}
		}
		tty_set(tty.cursor_x, tty.cursor_y, ' ', tty.fg, tty.bg);
	} else if (ch == '\t') {
		tty.cursor_x += 8 - (tty.cursor_x % 8);
		if (tty.cursor_x >= tty.width) {
			tty.cursor_x = 0;
			if (++tty.cursor_y == tty.height) {
				tty_scroll(1);
			}
		}
	} else if (ch == '\r') {
		tty.cursor_x = 0;
	} else {
		tty_set(tty.cursor_x, tty.cursor_y, (char) ch, tty.fg, tty.bg);
		if (++tty.cursor_x >= tty.width) {
			tty.cursor_x = 0;
			if (++tty.cursor_y >= tty.height) {
				tty_scroll(1);
			}
		}
	}
	return ch;
}

int tty_putchar(int c) {
	out8(0x3F8, c);
	if (c == '\n')
		out8(0x3F8, '\r');
	struct tty_ansi_parser *parser = &tty.ansi_parser;

	switch (parser->state) {
		case ANSI_ESC:
			if (c == '\033') {
				parser->state = ANSI_BRACKET;
				parser->index = 0;
				parser->stack[parser->index].value = 0;
				parser->stack[parser->index].empty = true;
			} else {
				tty_append(c);
			}
			break;
		case ANSI_BRACKET:
			if (c == '[') {
				parser->state = ANSI_ATTR;
			} else {
				parser->state = ANSI_ESC;
				tty_append(c);
			}
			break;
		case ANSI_ATTR:
			if (isdigit(c)) {
				parser->stack[parser->index].value *= 10;
				parser->stack[parser->index].value += (c - '0');
				parser->stack[parser->index].empty = false;
			} else {
				if (parser->index < 8) {
					parser->index++;
				}

				parser->stack[parser->index].value = 0;
				parser->stack[parser->index].empty = true;
				parser->state = ANSI_ENDVAL;
			}
			break;
		default:
			break;
	}

	if (parser->state == ANSI_ENDVAL) {
		if (c == ';') {
			parser->state = ANSI_ATTR;
			return c;
		} else if (c == 'm') {
			for (int i = 0; i < parser->index; i++) {
				if (parser->stack[i].empty || parser->stack[i].value == 0) {
					tty.fg = VGA_WHITE;
					tty.bg = VGA_BLACK;
				} else {
					int attr = parser->stack[i].value;
					if (attr >= 30 && attr <= 37)
						tty.fg = attr - 30;
					if (attr >= 90 && attr <= 97)
						tty.fg = attr - 82;
					if (attr >= 40 && attr <= 47)
						tty.bg = attr - 40;
					if (attr >= 100 && attr <= 107)
						tty.bg = attr - 92;
				}
			}
		} else if (c == 'H') {
			if (parser->stack[0].empty)
				tty.cursor_y = 0;
			else if (parser->stack[0].value > (int) tty.height)
				tty.cursor_y = tty.height - 1;
			else
				tty.cursor_y = parser->stack[0].value - 1;
			if (parser->stack[1].empty)
				tty.cursor_x = 0;
			else if (parser->stack[1].value > (int) tty.width)
				tty.cursor_x = tty.width - 1;
			else
				tty.cursor_x = parser->stack[1].value - 1;
		}
		parser->state = ANSI_ESC;
	}
	return c;
}

int tty_puts(const char *str) {
	int i = 0;
	while (str[i])
		tty_putchar(str[i++]);
	tty_cursor_update();
	return i;
}

void tty_cursor_set(size_t x, size_t y) {
	uint16_t pos = y * tty.width + x;
 
	out8(0x3D4, 0x0F);
	out8(0x3D5, (uint8_t) (pos & 0xFF));
	out8(0x3D4, 0x0E);
	out8(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void tty_cursor_update(void) {
	tty_cursor_set(tty.cursor_x, tty.cursor_y);
}
