#include "syscall.h"
#include "stdio.h"
#include "string.h"

long int ret;

int main(void) {
  printf("Displaying queue information : \n");
  info_queue();
}
