#include "../ulib/syscall.h"
#include "stdio.h"
#include "string.h"

long int ret;

int main(void) {
  printf("Welcome to our small os kernel \n");
  printf("In order to view the list of programs type : show_programs \n");
  printf("In order to run a program type its name directly into the kernel \n");
  printf("For example : type test0 \n");
  printf("If you want to see the currently running programs type : ps \n");
  printf("We hope you that you like our kernel :)\n");
}
