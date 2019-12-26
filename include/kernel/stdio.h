#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H 1

#include <stdarg.h>

int printf(const char*, ...);
int sprintf(char*, const char*, ...);
int vprintf(const char *, va_list);
int vsprintf(char*, const char*, va_list);

int putchar(int);
int puts(const char*);

#endif
