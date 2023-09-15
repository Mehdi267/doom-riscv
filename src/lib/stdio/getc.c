#include "stdio.h"
#include "unistd.h"

int fgetc(int fd){
  int ch;
  if (read(fd, &ch, 1) == 1){
    return ch;
  }else{
    return EOF;
  }
}

int getchar(void)
{
	return (fgetc(stdin));
}