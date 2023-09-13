#pragma once

#include <stdarg.h>
#include <stddef.h>

#define stdin 0
#define stdout 1
#define stderr 2


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
void	 perror(const char *);
int dprintf(int fd, const char *format, ...);
//######################################################
//##   Utility functions ##
//######################################################

/**
 * @brief Prints the the area pointed at by data 
 * and lite by the size parameter in binary
 * @param data a pointer to the area that we wish to display
 * @param size how many bytes we wish to display
 */
void printb(const void* data, size_t size);

/**
 * @brief Prints the bytes a block(equal to the size param)
 * of data starting at the pointer data. 
 * @param data the location of the block 
 * @param size the size of the block
 */
void print_block(void* data, size_t size);
