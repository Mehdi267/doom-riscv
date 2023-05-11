#include "../ulib/syscall.h"
#include "stdio.h"
#include "string.h"

int main(void *arg) {
  long int fid = (long int)arg;
  printf("fid = %li", fid);
  char ch;
  cons_echo(0);
  while(1) {
    cons_read(&ch, 1);
    psend(fid, (int)ch);
    /* printf("charater read was: %c\n", ch); */
  }
}
