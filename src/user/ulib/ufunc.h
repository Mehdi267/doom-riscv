#ifndef ___PERSONAL_USER_FUNC___
#define ___PERSONAL_USER_FUNC___
// #define NULL ((void*)0)
#include "stdio.h"
#define EOF -1

int per_fgetc(int fd);
char* my_fgets(char* buffer, int size, int fd);

#endif