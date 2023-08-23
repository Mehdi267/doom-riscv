#include <stdio.h>
#include "stdlib.h"

// static int tab[10000000] = {0};  // Initialize all elements to 0
// static int tab3[10000000];
int tab4[100000];
extern char _text_start_app[];
extern char _text_end_app[];
extern char _bss_start_app[];
extern char _bss_end_app[];
extern char _data_start_app[];
extern char _data_end_app[];

int main(int argc, char* argv[]){
  printf(" _text_start_app[] = %p\n", _text_start_app);
  printf(" _text_end_app[] = %p\n", _text_end_app);
  printf(" _bss_start_app[] = %p\n", _bss_start_app);
  printf(" _bss_end_app[] = %p\n", _bss_end_app);
  printf(" _data_start_app[] = %p\n", _data_start_app);
  printf(" _data_end_app[] = %p\n", _data_end_app);
  // printf(" tab pointer is equal to %p\n", tab);
  for (int i = 0; i < 100000; i++){
  //   tab3[i] = i;
      tab4[i] = i;
    // tab[i] = i;
   }
    // (void)tab4;
    // (void)tab3;
    // (void)tab;
  // int buf[4095]; 
  // (void)buf;
  // printf("run with success\n");
  // printf("add of malloc %p\n", malloc);
  // printf("before malloc\n");
  void *x = malloc(10);
  (void)x;
  printf("x pointer is equal to %p\n", x);
  // printf("adter malloc");
  // *(int*)add = 10;
  return 0;
}
