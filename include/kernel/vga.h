#ifndef _VGA_H
#define _VGA_H 1

#include <stdint.h>

enum vga_color {
	VGA_BLACK,
	VGA_RED,
	VGA_GREEN,
	VGA_YELLOW,
	VGA_BLUE,
	VGA_MAGENTA,
	VGA_CYAN,
	VGA_WHITE,
	VGA_BRIGHT_BLACK,
	VGA_BRIGHT_RED,
	VGA_BRIGHT_GREEN,
	VGA_BRIGHT_YELLOW,
	VGA_BRIGHT_BLUE,
	VGA_BRIGHT_MAGENTA,
	VGA_BRIGHT_CYAN,
	VGA_BRIGHT_WHITE,
};

struct vga_palette_entry {
	uint8_t index;
	uint8_t red, green, blue;
} __attribute__ ((packed));

void vga_init();

#endif
