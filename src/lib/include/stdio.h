#pragma once

#include <stdarg.h>
#include <stddef.h>


int printf(const char *, ...);
int puts(const char *);
int sprintf(char *, const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int vprintf(const char *, va_list);
int vsprintf(char *, const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
#define PRINT_GREEN(format, ...) printf("\033[0;32m" format "\033[0m", ##__VA_ARGS__)
#define PRINT_RED(format, ...) printf("\033[0;31m" format "\033[0m", ##__VA_ARGS__)
int isspace(int c);
int isdigit(int c);
int atoi(const char *str);
#define fprintf(f, ...) printf(__VA_ARGS__)
