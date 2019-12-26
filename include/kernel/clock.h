#ifndef _CLOCK_H
#define _CLOCK_H 1

#include <stdint.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define CENTURY 20

struct time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

void clock_init();
void get_time(struct time*);
void sleep();
uint64_t get_milliseconds();

#endif
