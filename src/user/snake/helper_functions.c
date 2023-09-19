#include "helper_functions.h"
#include "stdio.h"
#include "syscall.h"
#include "snake.h"
#define LINES 20
#define COLUMNS 20

#define MEMORY_SIZE 1000
char memory_pool[MEMORY_SIZE];

size_t offset = 0;
void * malloc(size_t size) {
  if (offset + size > MEMORY_SIZE) {
    printf("segmetation fault \n");
    exit(-1);
  }
  char * res = memory_pool + offset;
  offset += size;
  return res;
}

int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

void random_case(position_t * p) {
  p->j = mod(current_clock() ^ 0x8172f162019a89ff, COLUMNS);
  p->i = mod(p->j ^ 0xf22af2f8849e1b15, LINES);
}
