#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

void* memcpy(void*, const void*, size_t);
void* memset(void*, int, size_t);

size_t strlen(const char*);
void strncpy(char*, const char*, size_t);
void strcpy(char*, const char*);
char* strdup(const char*);
int strcmp(const char*, const char*);
char* strrchr(const char*, int);

#endif
