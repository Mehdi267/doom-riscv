#ifndef HELPER_FUNCTIONS_H_
#define HELPER_FUNCTIONS_H_

#include "stddef.h"
#include "snake.h"

void * malloc(size_t size);
int mod(int a, int b);
void random_case(position_t * p);
extern size_t offset;

#endif // HELPER_FUNCTIONS_H_
