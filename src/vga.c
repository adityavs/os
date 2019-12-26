#include "kernel/vga.h"

#include "kernel/io.h"

struct vga_palette_entry palette[16] = {
	{ 0x00,  1,  1,  1 }, // Black
	{ 0x01, 32,  1,  1 }, // Red
	{ 0x02,  1, 32,  1 }, // Green
	{ 0x03, 16, 16,  1 }, // Yellow
	{ 0x04,  1,  1, 32 }, // Blue
	{ 0x05, 32,  1, 32 }, // Magenta
	{ 0x14,  1, 32, 32 }, // Cyan
	{ 0x07, 32, 32, 32 }, // White
	{ 0x38, 16, 16, 16 }, // Bright Black
	{ 0x39, 63,  1,  1 }, // Bright Red
	{ 0x3A,  1, 63,  1 }, // Bright Green
	{ 0x3B, 63, 63,  1 }, // Bright Yellow
	{ 0x3C,  1,  1, 63 }, // Bright Blue
	{ 0x3D, 63,  1, 63 }, // Bright Magenta
	{ 0x3E,  1, 63, 63 }, // Bright Cyan
	{ 0x3F, 63, 63, 63 }, // Bright White
};

void vga_init() {
	while ((inb(0x03DA) & 0x08));
	while (!(inb(0x03DA) & 0x08));
	for (uint8_t i = 0; i < 16; i++) {
		outb(0x03C8, palette[i].index);
		outb(0x03C9, palette[i].red);
		outb(0x03C9, palette[i].green);
		outb(0x03C9, palette[i].blue);
	}
}
