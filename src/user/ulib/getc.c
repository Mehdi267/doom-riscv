#include "syscall.h"
#include "ufunc.h"

int per_fgetc(int fd){
  unsigned char ch;
  return (read(fd, &ch, 1) == 1) ? (int)ch : EOF;
}