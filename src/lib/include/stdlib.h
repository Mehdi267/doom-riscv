#pragma once

#include <stddef.h>
#include <stdint.h>

__attribute__((noreturn)) void abort(void);
__attribute__((noreturn)) void exit(int status);

//######################################################
//##   Memory Management ##
//######################################################

/*
* @brief this macro is meant to be used in function witch returns -1 on failure
*/
#define secmalloc(p, n)                                                        \
  if (n >= 2147483647){                                                        \
    return -1;                                                                 \
  }                                                                            \
  p = malloc(n);                                                               \
  if (p == NULL)                                                               \
  return -1

void* malloc(size_t size);
void* calloc(size_t n_elements, size_t element_size);
void *realloc(void *ptr, size_t size);
void* memalign(size_t alignment, size_t size);
void free(void* ptr);
long int strtol(const char *nptr, char **endptr, int base);
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
