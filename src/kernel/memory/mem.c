/*
 * Copyright (C) 2005 -- Simon Nieuviarts
 * Copyright (C) 2012 -- Damien Dejean <dam.dejean@gmail.com>
 *
 * Kernel memory allocator.
 */

#include "mem.h"
#include "stddef.h"
#include "stdio.h"
#include "process/process.h"

/* Heap boundaries */
extern char _heap_start[];
extern char _heap_end[];
static char *curptr = _heap_start;

//This method will not be called by the directly 
//kernel code but by the malloc function,
//It handles all of the dynamic memory allocation
/* Trivial sbrk implementation */
void *sbrk(ptrdiff_t diff)
{	
 	// debug_print_memory("[brk called ] curptr = %p \n", curptr);
 	// printf("[brk called ] curptr = %p \n", curptr);
	if (diff<0){
    printf("diff = %ld\n", diff);
  }
  char *s = curptr;
	char *c = s + diff;
	if ((c < curptr) || (c > _heap_end)) return ((void*)(-1));
	curptr = c;
	return s;
}

