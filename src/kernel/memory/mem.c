/*
 * Copyright (C) 2005 -- Simon Nieuviarts
 * Copyright (C) 2012 -- Damien Dejean <dam.dejean@gmail.com>
 *
 * Kernel memory allocator.
 */

#include "mem.h"
#include "stddef.h"

/* Heap boundaries */
extern char _heap_start[];
extern char _heap_end[];
static char *curptr = _heap_start;

/* Trivial sbrk implementation */
void *sbrk(ptrdiff_t diff)
{
	char *s = curptr;
	char *c = s + diff;
	if ((c < curptr) || (c > _heap_end)) return ((void*)(-1));
	curptr = c;
	return s;
}

