#include "syscall.h"
#include "ufunc.h"

int per_fgetc(int fd){
  int ch;
  if (read(fd, &ch, 1) == 1){
    return ch;
  }else{
    return EOF;
  }
}