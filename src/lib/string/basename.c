#include "string.h"
#include "stdlib.h"

char *basename(const char *path) {
  char *base = strrchr(path, '/');
  return (base != NULL) ? (base + 1) : (char *)path;
}