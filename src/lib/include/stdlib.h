#pragma once

#include <stddef.h>

__attribute__((noreturn)) void abort(void);
__attribute__((noreturn)) void exit(int status);

/*
* @brief this macro is meant to be used in function witch returns -1 on failure
*/
#define secmalloc(p, n)                                                        \
  p = malloc(n);                                                               \
  if (p == NULL)                                                               \
  return -1

void* malloc(size_t size);
void* calloc(size_t n_elements, size_t element_size);
void* memalign(size_t alignment, size_t size);
void free(void* ptr);
long int strtol(const char *nptr, char **endptr, int base);
