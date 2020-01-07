#include "kernel/clock.h"

#include <stdbool.h>
#include <stdio.h>

#include "kernel/interrupts.h"
#include "kernel/io.h"

volatile uint64_t sleep_timer = 0;
volatile uint64_t milliseconds = 0;

void pit_callback(struct isr_stack s) {
	(void) s;
	milliseconds++;
	if (sleep_timer > 0)
		sleep_timer--;
}

void clock_init() {
	interrupts_set_handler(32, &pit_callback);
	uint64_t divisor = 1193; // 1193180Hz^2 / 1000Hz;
	out8(0x43, 0x36);
	out8(0x40, (uint8_t) (divisor & 0xFF));
	out8(0x40, (uint8_t) ((divisor >> 8) & 0xFF));
}

bool is_cmos_updating() {
    out8(CMOS_ADDRESS, 0x0A);
    return (in8(CMOS_DATA) & 0x80);
}

uint8_t get_register(int r) {
    out8(CMOS_ADDRESS, r);
    return in8(CMOS_DATA);
}

#define BCD_TO_INT(val) (((val) >> 4) * 10 + ((val) & 0xF))
void get_time(struct time *time) {
	while (is_cmos_updating());
	time->second = BCD_TO_INT(get_register(0));
    time->minute = BCD_TO_INT(get_register(2));
    time->hour = BCD_TO_INT(get_register(4));
    time->day = BCD_TO_INT(get_register(7));
    time->month = BCD_TO_INT(get_register(8));
	time->year = BCD_TO_INT(get_register(9));
}

void sleep(uint64_t ms) {
	sleep_timer = ms;
	while (sleep_timer > 0);
}

uint64_t get_milliseconds() {
	return milliseconds;
}
